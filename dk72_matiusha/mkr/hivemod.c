#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/debugfs.h>
#include <linux/types.h>
#include <linux/rbtree.h>


MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("thodnev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

u64 beeval = 12;
struct dentry *ddir = NULL, *dfile = NULL;
#define  MODULE_CLASS_NAME  "hive_cdev_class"
static struct class *hive_class = NULL;
#define TWERK _IO('i', 1)
static struct rb_root tree = RB_ROOT;
/**
 * MOD_DEBUG(level, fmt, ...) - module debug printer
 * @level: printk debug level (e.g. KERN_INFO)
 * @fmt: printk format
 *
 * Prints message using printk(), adding module name
 */
#define MOD_DEBUG(level, fmt, ...) \
	{printk(level "%s: " fmt "\n", THIS_MODULE->name,##__VA_ARGS__);}
// How it works? What's inside THIS_MODULE?
// Are there any analogs?

/**
 * struct alloc_status - bit field, stores resource allocation flags
 * @dev_created: character device has been successfully created
 */
struct alloc_status {
	unsigned long dev_created : 1;
	unsigned long cdev_added : 1;
	unsigned long class_created : 1;
	unsigned long dev_registered : 1;
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

LIST_HEAD(hive_flist);

static const char magic_phrase[] = "Wow, we made these bees TWERK !";

static char *devname = THIS_MODULE->name;
module_param(devname, charp, 0);
MODULE_PARM_DESC(devname, "Name as in VFS. Defaults to module name");
static int major = 0;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major number. Defaults to 0 (automatic allocation)");
static int buffsize = 2 * sizeof(magic_phrase);
module_param(buffsize, int, 0);
MODULE_PARM_DESC(buffsize, "Char buffer size. Defaults to 2 * sizeof magic_phrase");

dev_t hive_dev = 0;	// Stores our device handle
static struct cdev hive_cdev; // scull-initialized

/**
 * hive_flist_new() - creates list item having buffer
 * @buffer_size: numer of characters in buffer
 */
static inline struct hive_flist_item *hive_flist_new(unsigned long buffer_size)
{
	// (!) here's where kernel memory (probably containing secrets) leaks
        // to userspace...
        // TODO: fix to make it zero'ed first
		// DONE: changed to kzalloc
	char *buf = kzalloc(sizeof(*buf) * buffer_size, GFP_KERNEL);
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

static int rb_insert(struct rb_root *root, struct hive_flist_item *item)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;
	/* Figure out where to put new node */
	while (*new) {
		struct hive_flist_item *this = container_of(*new, struct hive_flist_item, node);
		int result = memcmp(item->file, this->file, sizeof(item->file));
		parent = *new;
		if (result < 0) {
			new = &((*new)->rb_left);
		} else if (result > 0) {
			new = &((*new)->rb_right);
		} else
			return 1;
	}
	/* Add new node and rebalance tree. */
	rb_link_node(&item->node, parent, new);
	rb_insert_color(&item->node, root);
	return 0;
}


/**
 * hive_flist_rm() - deletes item from list and frees memory
 * @item: list item
 */
static inline void hive_flist_rm(struct hive_flist_item *item)
{
	if (NULL == item)
		return;
	rb_erase(&item->node, &tree);
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
	struct hive_flist_item *item;
	struct rb_node *node = root->rb_node;
	while (node) {
		struct hive_flist_item *data = container_of(node, struct hive_flist_item, node);
		int result = memcmp(data->file, file, sizeof(data->file));
		if (result < 0) {
			node = node->rb_left;
		} else if (result > 0) {
			node = node->rb_right;
		} else {
			return data;
		}
	}
	return NULL;
	// not found
	MOD_DEBUG(KERN_ERR, "Expected list entry not found %p", file);
	return NULL;
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
	rb_insert(&tree, item);
	MOD_DEBUG(KERN_DEBUG, "New file entry %p created", file);
	return 0;
}

/**
 * cdev_release() - file close() callback
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 */
static int cdev_release(struct inode *inode, struct file *file)
{
	struct hive_flist_item *item = hive_flist_get(&tree, file);
	if (NULL == item)
		return -EBADF;
	// remove item from list and free its memory
	hive_flist_rm(item);
	MOD_DEBUG(KERN_DEBUG, "File entry %p unlinked", file);
	return 0;
}

/**
 * cdev_read() - called on file read() operation
 * @file: VFS file opened by a process
 * @buf:
 * @count:
 * @loff:
 */
static ssize_t cdev_read(struct file *file, char __user *buf, 
			 size_t count, loff_t *loff)
{
	struct hive_flist_item *item = hive_flist_get(&tree, file);
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer read logic. Make sure seek operations work
    	//       correctly
	//       Be careful not to access array above bounds
	if(*loff >= item->length) {
		MOD_DEBUG(KERN_DEBUG, "Read pointer above file size");
		return -EINVAL;
	}
	if(*loff + count > item->length) {
		count = item->length - *loff;
	}

	if(copy_to_user(buf, item->buffer + *loff, count)) {
		MOD_DEBUG(KERN_DEBUG, "Failed to read file");
		return -EFAULT;
	}
	*loff += count;
	return count;
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
	struct hive_flist_item *item = hive_flist_get(&tree,file);
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer write logic. Make sure seek operations work
    	//       correctly
	//       Be careful not to access array above bounds
	char *buf_m = kzalloc(sizeof(*buf_m) * count, GFP_KERNEL);
	if (NULL == buf_m) {
		MOD_DEBUG(KERN_DEBUG, "No mem");
		return -ENOMEM;
	}
	item->buffer = buf_m;

	if(copy_from_user(item->buffer + *loff, buf, count) != 0) {
		MOD_DEBUG(KERN_DEBUG, "Failed to write file");
		return -EFAULT;
	}
	*loff += count;

	if(item->length < *loff) {
		item->length = *loff;
	}
	return count;
	return 0;
}

/**
 * cdev_lseek
 * @file: file pointer
 * @offset: requested offset to be set the file
 * @origin: SEEK_SET, SEEK_CUR, SEEK_END
 */
static loff_t cdev_lseek(struct file *file, loff_t offset, int origin)
{
	struct hive_flist_item *item = hive_flist_get(&tree, file);
	if (NULL == item)
		return -EBADF;
	loff_t newpos;
	switch(origin) {
	case SEEK_SET:
		newpos = offset;
		break;
	case SEEK_CUR:
		newpos = offset + file->f_pos;
		break;
	case SEEK_END:
		newpos = item->length + offset;
		break;
	default:
		return -EINVAL;
		break;
	}
	if(newpos < 0) {
		return -EINVAL;
	}
	file->f_pos = newpos;
	return newpos;
}


static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct hive_flist_item *item = hive_flist_get(&tree, file);
	if (NULL == item)
		return -EBADF;
	switch(cmd) {
		case TWERK:
			MOD_DEBUG(KERN_INFO, "TWERK:");
			strcpy(item->buffer, magic_phrase);
			break;
		default:
			return -ENOTTY;
	}
	return 0;
}

// This structure is partially initialized here
// and the rest is initialized by the kernel after call
// to cdev_init()
// TODO: add ioctl to append magic phrase to buffer conents to
//       make these bees twerk
// TODO: add ioctl to select buffer size
static struct file_operations hive_fops = {
	.open =    &cdev_open,
	.release = &cdev_release,
	.read =    &cdev_read,
	.write =   &cdev_write,
	.unlocked_ioctl = &cdev_ioctl,
	.llseek = &cdev_lseek,
	// required to prevent module unloading while fops are in use
	.owner =   THIS_MODULE,
};


static void module_cleanup(void)
{
	// notice: deallocations happen in *reverse* order
	if(alloc_flags.dev_registered) {
		device_destroy(hive_class, hive_dev);
	}
	if(alloc_flags.class_created)  {
		class_unregister(hive_class);
		class_destroy(hive_class);
	}
	if (alloc_flags.cdev_added) {
		cdev_del(&hive_cdev);
	}
	if (alloc_flags.dev_created) {
		unregister_chrdev_region(hive_dev, 1);
	}
	// paranoid cleanup (afterwards to ensure all fops ended)
	struct hive_flist_item *item;
	struct rb_node *rbp = rb_first(&tree);
	struct rb_node *rb_l = rb_last(&tree);
	while(rbp != rb_l) {
		item = rb_entry_safe(rbp, struct hive_flist_item, node);
		hive_flist_rm(item);
		rbp = rb_next(rbp);
	}
}

static int __init cdevmod_init(void)
{
	int err = 0;
	ddir = debugfs_create_dir("hivemod", NULL);
	//dfile = 
	debugfs_create_u64("beeval", 0777, ddir, &beeval);
	
	if (0 == major) {
		// use dynamic allocation (automatic)
		err = alloc_chrdev_region(&hive_dev, 0, 1, devname);
	} else {
		// stick with what user provided
		hive_dev = MKDEV(major, 0);
		err = register_chrdev_region(hive_dev, 1, devname);
	}

	if (err) {
		MOD_DEBUG(KERN_ERR, "%s dev %d create failed with %d",
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
	if ((hive_class = class_create(THIS_MODULE, "hive_class")) == NULL) {
		unregister_chrdev_region(hive_dev, 1);
		return -1;
	}
	alloc_flags.class_created = 1;

	if (device_create(hive_class, NULL, hive_dev, NULL, "hive_dev") == NULL) {
		class_destroy(hive_class);
		unregister_chrdev_region(hive_dev, 1);
		return -1;
	}
	alloc_flags.dev_registered = 1;
	MOD_DEBUG(KERN_DEBUG, "This hive has %lu bees", 2 + jiffies % 8);
	// TODO: add stuff here to make module register itself in /dev
	return 0;

err_handler:
	module_cleanup();
	return err;
}
 
static void __exit cdevmod_exit(void)
{
	module_cleanup();
	MOD_DEBUG(KERN_DEBUG, "%d bees", (int)beeval);
	MOD_DEBUG(KERN_DEBUG, "All honey reclaimed");
	debugfs_remove_recursive(ddir);
}
 
module_init(cdevmod_init);
module_exit(cdevmod_exit);
