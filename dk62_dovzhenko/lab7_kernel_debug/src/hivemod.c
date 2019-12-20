/* Based on code made by thodnev.
 * This code was implemented by:  
 *  -Olexander Ostrianko
 *  -Maxim Salim
 *  -Dana Dovzhenko
 * CAUTION: Don`t try to read and also to gain insight this code without a
 * cigarette. It can damage your imagine!
 */


#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>	// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h> 
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/rbtree.h>
#include <linux/debugfs.h>

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("AlexOstrianko, DanaDov, maksimo0");
MODULE_VERSION("0.2");
MODULE_LICENSE("Dual MIT/GPL");

/**
 * MOD_DEBUG(level, fmt, ...) - module debug printer
 * @level: printk debug level (e.g. KERN_INFO)
 * @fmt: printk format
 *
 * Prints message using printk(), adding module name
 */
#define MOD_DEBUG(level, fmt, ...) \
	{printk(level "%s: " fmt "\n", MODNAME, ##__VA_ARGS__);}
// How it works? What's inside THIS_MODULE?
// Are there any analogs?

#define DIR "hivefold"
#define FILE "hive"


/**
 * struct alloc_status - bit field, stores resource allocation flags
 * @dev_created: character device has been successfully created
 */
struct alloc_status {
	unsigned long dev_created : 1;
	unsigned long cdev_added : 1;
	unsigned long class_created : 1;
	unsigned long cdev_registered : 1;
};
// start with everything non-done
static struct alloc_status alloc_flags = { 0 };


/**
 * struct hive_flist_item - stores data for each descriptor
 * @list: fields to link the list
 * @file: created on open(), removed on close()
 *        changes during file operations, but ptr stays the same
 * @buffer: memory we allocate for each file
 * @length: buffer size
 * @rdoffset: reader offset
 * @wroffset: writer offset
 *
 * This implementation is not optimal as it imposes linear O(N)
 * lookup through list.
 * TODO: change this to proper associative array or tree
 */
struct hive_flist_item {
	struct rb_node node;
	struct file *file;
	char *buffer;
	long length;
	long rdoffset;
	long wroffset;
};

struct rb_root the_root = RB_ROOT;

static const char magic_phrase[] = "Wow, we made these bees TWERK!";

#if !defined(MODULE)
  #define MODNAME "hivemod"
#else
  #define MODNAME "THIS_MODULE->name"
#endif

//static char *devname = try_module_get(THIS_MODULE);
static char *devname = MODNAME;
module_param(devname, charp, 0);
MODULE_PARM_DESC(devname, "Name as in VFS. Defaults to module name");
static int major = 0;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major number. Defaults to 0 (automatic allocation)");
static int buffsize = 2 * sizeof(magic_phrase);
module_param(buffsize, int, 0);
MODULE_PARM_DESC(buffsize, "Char buffer size. Defaults to 2 * sizeof magic_phrase");

dev_t hive_dev = 0;	// Stores our device handle
static struct cdev hive_cdev;	// scull-initialized
static struct class *class_char = NULL;	// Global variable for the device class

struct dentry *debugfs_folder, *debugfs_file;

#define BEES_TWERK _IOW('a', 0, char *)
#define SEL_BUF_SIZE _IOW('a', 1, int *)

/**
 * hive_flist_new() - creates list item having buffer
 * @buffer_size: numer of characters in buffer
 */
static inline struct hive_flist_item *hive_flist_new(unsigned long buffer_size)
{
	// (!) here's where kernel memory (probably containing secrets) leaks
        // to userspace...
        // TODO: fix to make it zero'ed first
	char *buf = kzalloc(sizeof(*buf) * buffsize, GFP_KERNEL);
	if (NULL == buf)
		return NULL;

	struct hive_flist_item *item = kzalloc(sizeof *item, GFP_KERNEL);
	if (NULL == item) {
		kfree(buf);	// avoid mem leaks
		return NULL;
	}
	
	item->buffer = buf;
	item->length = buffer_size;
	item->rdoffset = 0;
	item->wroffset = 0;
	return item;
}

void tree_print(struct hive_flist_item *rootp)
{
	if (NULL != rootp) {
		
	}
}

void hive_tree_rm(struct hive_flist_item *item)
{
	if (NULL == item)
		return;
	rb_erase(&item->node, &the_root);
	kfree(item->buffer);
	kfree(item);
}

/**
 * hive_flist_get - searches the list
 * @file: field of the list
 *
 * Return: item having the field or NULL if not found
 */
static struct hive_flist_item *hive_flist_get(struct rb_root *root, struct file *file)
{
	struct rb_node *node = root->rb_node;
	while (node) {
		struct hive_flist_item *strp = rb_entry(node, 
			struct hive_flist_item, node);
		if (strp->file > file)
			node = node->rb_left;
		else if (strp->file < file)
			node = node->rb_right;
		else
			return strp;
	}
	return NULL;
}

void ftree_add (struct hive_flist_item *item, struct rb_root *root)
{
	struct rb_node **link = &root->rb_node, *parent;
	struct file *val4cmp = item->file;
	while(*link) {
		parent = *link;
		struct hive_flist_item *strp = rb_entry(parent, struct hive_flist_item,
			node);
		if (strp->file > val4cmp)
			link = &(*link)->rb_left;
		else
			link = &(*link)->rb_right;
	}
	rb_link_node(&item->node, parent, link);
	rb_insert_color(&item->node, root);
}

// For more, see LKD 3rd ed. chapter 13
/**
 * cdev_open() - callback for open() file operation
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 *
 * Allocates memory, creates fd entry and adds it to linked list
 */
static int cdev_open(struct inode *inode, struct file *file)
{
	struct hive_flist_item *item = hive_flist_new(buffsize);
	if (NULL == item) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		return -ENOMEM;
	}
	// fill the rest
	item->file = file;
	ftree_add(item, &the_root);

	//MOD_DEBUG(KERN_DEBUG, "New file entry %p created", file);
	return 0;
}

/**
 * cdev_release() - file close() callback
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 */
static int cdev_release(struct inode *inode, struct file *file)
{
	struct hive_flist_item *item = hive_flist_get(&the_root, file);
	if (NULL == item)
		return -EBADF;
	// remove item from list and free its memory
	//hive_tree_rm(item);
	//MOD_DEBUG(KERN_DEBUG, "File entry %p unlinked", file);
	return 0;
}

/**
 * cdev_read() - called on file read() operation
 * @file: VFS file opened by a process
 * @buf:
 * @count:
 * @loff:
 */

static ssize_t cdev_read(struct file *file, char __user *buf, size_t count, 
			loff_t *loff)
{
	struct hive_flist_item *item = hive_flist_get(&the_root, file);
	if (NULL == item)
		return -EBADF;

	int max_leng;
	int bytes_to_read;
	int read_b;
	
	max_leng = buffsize - *loff;
	if (max_leng > count)
		bytes_to_read = count;
	else
		bytes_to_read = max_leng;
	if (bytes_to_read == 0)
		printk(KERN_INFO "The end of device\n");
	read_b = bytes_to_read - copy_to_user(buf, item->buffer + *loff, 
		bytes_to_read);
	*loff += read_b;
	return read_b;
}

/**
 * cdev_write() - callback for file write() operation
 * @file: VFS file opened by a process
 * @buf:
 * @count:

 * @loff:
 */

static ssize_t cdev_write(struct file *file, const char __user *buf, 
			size_t count, loff_t *loff)
{
	struct hive_flist_item *item = hive_flist_get(&the_root, file);
	if (NULL == item)
		return -EBADF;
	int max_leng;
	int bytes_to_write;
	int write_b;
	max_leng = buffsize - *loff;
	if (max_leng > count)
		bytes_to_write = count;
	else
		bytes_to_write = max_leng;
	
	if (bytes_to_write == 0) {
		printk(KERN_INFO "END\n");
	}
	
	printk(KERN_INFO "line to write# %s\n", buf);
	write_b = bytes_to_write - copy_from_user(item->buffer + *loff, buf, 
		bytes_to_write);
	//item->buffer[bytes_to_write] = '\0';
	printk(KERN_INFO "The end of device, device has been wrtitten %d", 
		write_b);
	*loff += write_b;
	return write_b;
}

static loff_t cdev_lseek(struct file *file, loff_t offset, int orig)
{
	loff_t new_offset = 0;
	switch(orig) {
		case SEEK_SET: 
			new_offset = offset; 
			break;
		case SEEK_CUR: 
			new_offset = file->f_pos + offset; 
			break;
		case SEEK_END: 
			new_offset = buffsize - offset; 
			break;
	}
	file->f_pos = new_offset;
	return new_offset;
}

static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{	
	struct hive_flist_item *item = hive_flist_get(&the_root, file);
	switch(cmd) {
		case BEES_TWERK:
			copy_from_user(item->buffer, (char *)arg, 
				strlen((char *)arg));
			printk(KERN_INFO "Magic phrase = %s\n", item->buffer);
			break;
		case SEL_BUF_SIZE:
			copy_from_user(&buffsize, (int *)arg, sizeof(buffsize));
			printk(KERN_INFO "Buffsize = %d\n", buffsize);	
			break;
	}
	return 0;
}

// This structure is partially initialized here
// and the rest is initialized by the kernel after call
// to cdev_init()
static struct file_operations hive_fops = {
	.open =			&cdev_open,
	.release =		&cdev_release,
	.read =			&cdev_read,
	.write =		&cdev_write,
	.llseek =		&cdev_lseek,
	.unlocked_ioctl =	&cdev_ioctl,
	// required to prevent module unloading while fops are in use
	.owner =		THIS_MODULE,
};

static int debugfs_open(struct inode *inode, struct file *file)
{	
	printk(KERN_INFO "FILE OPENED!\n");
	return 0;
}

static int debugfs_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "FILE RELEASED!\n");
	return 0;
}

static const char test_str[] = "Hello test str!";

static ssize_t debugfs_read(struct file *file, char __user *buf, size_t count, 
			loff_t *loff)
{
	struct hive_flist_item *item = rb_entry(the_root.rb_node, 
		struct hive_flist_item, node);
	if (NULL == item)
		return -EBADF;
	printk(KERN_INFO "file buff is# %s\n", item->buffer);

	int max_leng;
	int bytes_to_read;
	int read_b;
	
	max_leng = 16 - *loff;
	if (max_leng > count)
		bytes_to_read = count;
	else
		bytes_to_read = max_leng;
	if (bytes_to_read == 0)
		printk(KERN_INFO "The end of device\n");
	read_b = bytes_to_read - copy_to_user(buf, item->buffer + *loff, 
		bytes_to_read);
	*loff += read_b;
	return read_b;
}

static ssize_t debugfs_write(struct file *file, const char __user *buf, 
			size_t count, loff_t *loff)
{	
	struct hive_flist_item *item = rb_entry(the_root.rb_node, 
		struct hive_flist_item, node);
	if (NULL == item)
		return -EBADF;

	int max_leng;
	int bytes_to_write;
	int write_b;
	max_leng = buffsize - *loff;
	if (max_leng > count)
		bytes_to_write = count;
	else
		bytes_to_write = max_leng;
	
	if (bytes_to_write == 0) {
		printk(KERN_INFO "END\n");
	}
	
	printk(KERN_INFO "line to write# %s\n", buf);
	write_b = bytes_to_write - copy_from_user(item->buffer + *loff, buf, 
		bytes_to_write);
	printk(KERN_INFO "The end of device, device has been wrtitten %d", 
		write_b);
	*loff += write_b;
	return write_b;
}

static struct file_operations debugfs_fops = {
	.open =			&debugfs_open,
	.release =		&debugfs_release,
	.read =			&debugfs_read,
	.write =		&debugfs_write,
};

static void module_cleanup(void)
{
	if (alloc_flags.cdev_registered) {
		device_destroy(class_char, hive_dev);
	}
	if (alloc_flags.class_created) {
		class_unregister(class_char);
		class_destroy(class_char);
	}
	

	// notice: deallocations happen in *reverse* order
	if (alloc_flags.cdev_added) {
		cdev_del(&hive_cdev);
	}
	if (alloc_flags.dev_created) {
		unregister_chrdev_region(hive_dev, 1);
	}
	// paranoid cleanup (afterwards to ensure all fops ended)
	struct rb_root *root = &the_root;
	while (true) {
		if (NULL != root->rb_node) {
			struct hive_flist_item *delp = rb_entry(root->rb_node, 
				struct hive_flist_item, node);
			printk(KERN_INFO "paranoid: %s\n", delp->buffer);
			hive_tree_rm(delp);
		} else {
			break;
		}
	}
	debugfs_remove_recursive(debugfs_folder);
}

static int __init cdevmod_init(void)
{
	int err = 0;

	if (NULL == the_root.rb_node)
        	printk(KERN_INFO "root is NULL!");
	else
        	printk(KERN_INFO "root is something!");
	
	if (0 == major) {
		// use dynamic allocation (automatic)
		err = alloc_chrdev_region(&hive_dev, 0, 1, devname);
	} else {
		// stick with what user provided
		hive_dev = MKDEV(major, 0);
		err = register_chrdev_region(hive_dev, 1, devname);
	}

	if (err) {
		MOD_DEBUG(KERN_ERR, "%s dev %d create failed with %d\n",
			  major ? "Dynamic" : "Static",
		          major, err);
		goto err_handler;
	}
	
	

	alloc_flags.dev_created = 1;
	MOD_DEBUG(KERN_DEBUG, "%s dev %d:%d created",
		 major ? "Dynamic" : "Static",
	         MAJOR(hive_dev), MINOR(hive_dev));

	cdev_init(&hive_cdev, &hive_fops);
	// after call below the device becomes active
	// so all stuff should be initialized before
	if ((err = cdev_add(&hive_cdev, hive_dev, 1))) {
		MOD_DEBUG(KERN_ERR, "Add cdev failed with %d", err);
		goto err_handler;
	}
	alloc_flags.cdev_added = 1;
	// TODO: add stuff here to make module register itself in /dev

	 if ((class_char = class_create(THIS_MODULE, "class_char")) == NULL) {
        	printk(KERN_ERR "Class creation failed!");
		goto err_handler;
    	}
	alloc_flags.class_created = 1;

	if (device_create(class_char, NULL, hive_dev, NULL, "hive_dev") == NULL) {
		printk(KERN_ERR "Device creation failed!");
		goto err_handler;
    	}
	alloc_flags.cdev_registered = 1;

	
	debugfs_folder = debugfs_create_dir(DIR, NULL);
	debugfs_file = debugfs_create_file(FILE, 
		0777, debugfs_folder, NULL, &debugfs_fops);

	MOD_DEBUG(KERN_DEBUG, "This hive has %lu bees", 2 + jiffies % 8);

	return 0;

err_handler:
	module_cleanup();
	return err;
}
 
static void __exit cdevmod_exit(void)
{
	module_cleanup();
	MOD_DEBUG(KERN_DEBUG, "All honey reclaimed");
}
 
module_init(cdevmod_init);
module_exit(cdevmod_exit);

