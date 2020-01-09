//Some algoritms were taken from the Yaroslav Sokol
#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/rbtree.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("Artem Kyianytsia");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

/**
 * MOD_DEBUG(level, fmt, ...) - module debug printer
 * @level: printk debug level (e.g. KERN_INFO)
 * @fmt: printk format
 *
 * Prints message using printk(), adding module name
 */
#define MOD_DEBUG(level, fmt, ...) \
	{printk(level "%s: " fmt "\n", THIS_MODULE->name,##__VA_ARGS__);}


#define  MODULE_CLASS_NAME  "hive_cdev_class"

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
struct rb_root my_tree = RB_ROOT;

/**
 * struct ftree_item - stores data for each descriptor
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
 * change this to proper associative array or tree
 */
struct ftree_item {
	struct rb_node node;
	struct file *file;
	char *buffer;
	long length;
	long rdoffset;
	long wroffset;
};

static char *magic_phrase;
module_param(magic_phrase, charp, 0);
MODULE_PARM_DESC(magic_phrase, "Magic phrase");
static char *devname = THIS_MODULE->name;
module_param(devname, charp, 0);
MODULE_PARM_DESC(devname, "Name as in VFS. Defaults to module name");
static int major = 0;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major number. Defaults to 0 (automatic allocation)");
static int buffsize = 0;
module_param(buffsize, int, 0);
MODULE_PARM_DESC(buffsize, "Char buffer size. Defaults to 2 * sizeof magic_phrase");

dev_t hive_dev = 0;	// Stores our device handle
static struct cdev hive_cdev; // scull-initialized
static struct class *hive_class = NULL;

/**
 * tree_insert() - insert item to tree
 * @root:      pointer to root 
 * @hive_item: item of struct hive_item
 */
static int rb_insert(struct rb_root *root, struct ftree_item *item)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;
	/* Figure out where to put new node */
	while (*new) {
		struct ftree_item *this = container_of(*new, struct ftree_item, node);
		int result = memcmp(item->file, this->file, sizeof(struct file));
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
 * hive_flist_new() - creates list item having buffer
 *
 */
static inline struct ftree_item *ftree_new(void)
{
	struct ftree_item *item = kzalloc(sizeof *item, GFP_KERNEL);
	if (NULL == item) {
		return NULL;
	}
	item->buffer = NULL;
	item->length = 0;
	item->rdoffset = 0;
	item->wroffset = 0;
	return item;
}

/**
 * hive_flist_rm() - deletes item from list and frees memory
 * @item: list item
 */
static inline void ftree_rm(struct ftree_item *item)
{
	if (NULL == item)
		return;
	rb_erase(&item->node, &my_tree);
	kfree(item->buffer);
	kfree(item);
}

/**
 * hive_flist_get - searches the list
 * @file: field of the list
 *
 * Return: item having the field or NULL if not found
 */
static struct ftree_item *ftree_get(struct rb_root *root, struct file *file)
{
	struct rb_node *node = root->rb_node;
	while (node) {
		struct ftree_item *data = container_of(node, struct ftree_item, node);
		int result = memcmp(data->file, file, sizeof(file));
		if (result < 0) {
			node = node->rb_left;
		} else if (result > 0) {
			node = node->rb_right;
		} else {
			return data;
		}
	}
	return NULL;
}

/**
 * cdev_open() - callback for open() file operation
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 *
 * Allocates memory, creates fd entry and adds it to linked list
 */
static int cdev_open(struct inode *inode, struct file *file)
{
	struct ftree_item *item = ftree_new();
	if (NULL == item) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		return -ENOMEM;
	}
	// fill the rest
	item->file = file;
	if(!rb_insert(&my_tree, item)) {
		MOD_DEBUG(KERN_DEBUG, "New file entry %p created", file);
	} else {
		MOD_DEBUG(KERN_DEBUG, "New file not created");
	}
	return 0;
}

/**
 * cdev_release() - file close() callback
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 */
static int cdev_release(struct inode *inode, struct file *file)
{
	struct ftree_item *item = ftree_get(&my_tree, file);
	if (NULL == item)
		return -EBADF;
	// remove item from list and free its memory
	ftree_rm(item);
	MOD_DEBUG(KERN_DEBUG, "File entry %p unlinked", file);
	return 0;
}

/**
 * cdev_read() - called on file read() operation
 * @file: VFS file opened by a process
 * @buf:   buffer provided from userspace
 * @count: bytes requested to read in buffer
 * @loff:  current position in the file
 */
static ssize_t cdev_read(struct file *file, char __user *buf,
			 size_t count, loff_t *loff)
{
	struct ftree_item *item = ftree_get(&my_tree, file);
	if (NULL == item) {
		return -EBADF;
	}
	if(*loff >= item->length) {
		MOD_DEBUG(KERN_DEBUG, "Read pointer above file size");
		return -ENOMEM;
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
 * @file:  VFS file opened by a process
 * @buf:   buffer provided from userspace
 * @count: bytes requested to write from buffer
 * @loff:  current position in the file
 */
static ssize_t cdev_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *loff)
{
	struct ftree_item *item = ftree_get(&my_tree, file);
	if (NULL == item) {
		MOD_DEBUG(KERN_DEBUG, "Write ERROR");
		return -EBADF;
	}

	char *buf_m = kzalloc(sizeof(*buf_m) * count, GFP_KERNEL);
	if (NULL == buf_m) {
		MOD_DEBUG(KERN_DEBUG, "Write ERROR");
		return -EBADF;
	}
	item->buffer = buf_m;
	
	if(copy_from_user(item->buffer + *loff, buf, count) != 0) {
		MOD_DEBUG(KERN_DEBUG, "Failed to write file");
		return -EFAULT;
	}
	*loff += count;
	MOD_DEBUG(KERN_INFO, "buffer = %s | %li", item->buffer, strlen(item->buffer));

	if(item->length < *loff) {
		item->length = *loff;
	}
	return count;
}

/**
 * cdev_lseek - callback lseek (!test!)
 * @file: file pointer
 * @offset: requested offset to be set the file
 * @origin: SEEK_SET, SEEK_CUR, SEEK_END
 */
static loff_t cdev_llseek(struct file *file, loff_t offset, int origin)
{
	struct ftree_item *item = ftree_get(&my_tree, file);
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
		MOD_DEBUG(KERN_DEBUG, "Macross name is incorrect");
		return -EINVAL;
		break;
	}
	if(newpos < 0) {
		return -EINVAL;
	}
	file->f_pos = newpos;
	return newpos;
}

#define LENGTH _IOW('i', 0, int *)
#define BUFFER _IOW('i', 1, char *)

/**
 * cdev_ioctl - callback ioctl (!test!)
 * @file:        file pointer
 * @cmd:   BUFFER, LENGTH
 * @arg: perameter from userspace
 */
static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct ftree_item *item = ftree_get(&my_tree, file);
	if (NULL == item)
		return -EBADF;
	switch(cmd) {
	case BUFFER:
		MOD_DEBUG(KERN_INFO, "Flag BUFFER:");
		char *buf = kzalloc(sizeof(*buf) * item->length, GFP_KERNEL);
		if (NULL == buf) {
			MOD_DEBUG(KERN_DEBUG, "Write ERROR");
			return -EBADF;
		}
		item->buffer = buf;
		if(copy_from_user(item->buffer, (char *)arg, item->length) != 0) {
			MOD_DEBUG(KERN_DEBUG, "Failed to write file");
			return -EFAULT;
		}
		MOD_DEBUG(KERN_INFO, "BUFFER = %s", item->buffer);
		break;
	case LENGTH:
		MOD_DEBUG(KERN_INFO, "Flag LENGTH:");
		item->length = arg;
		MOD_DEBUG(KERN_INFO, "LENGTH = %li", item->length);
		break;
	default:
		return -ENOTTY;
	}
	return 0;
}

static struct file_operations hive_fops = {
	.open = &cdev_open,
	.release = &cdev_release,
	.read =	&cdev_read,
	.write = &cdev_write,
	.unlocked_ioctl = &cdev_ioctl,
	.llseek = &cdev_llseek,
	// required to prevent module unloading while fops are in use
	.owner = THIS_MODULE,
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
	struct ftree_item *item;
	struct rb_node *rbp = rb_first(&my_tree);
	struct rb_node *rb_l = rb_last(&my_tree);
	while(rbp != rb_l) {
		item = rb_entry_safe(rbp, struct ftree_item, node);
		ftree_rm(item);
		rbp = rb_next(rbp);
	}
}

static int __init cdevmod_init(void)
{
	int err = 0;
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
