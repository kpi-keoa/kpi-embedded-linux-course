#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include "ioctl.h"
#include <linux/debugfs.h>

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("thodnev & AMD_Inc.");
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

////////////////////////////////////////////////////////////////////////////////////////////////
//____________________________LAB7______________________________________________________________
////////////////////////////////////////////////////////////////////////////////////////////////


struct dentry *dirret;
struct debugfs_blob_wrapper *blob_d;
static u64 sum = 0;

static int add_write_op(void *data, u64 value)
{
     sum += value;
     return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(fops_debug, NULL, add_write_op, "%llu\n");

////////////////////////////////////////////////////////////////////////////////////////////////
//____________________________LAB7______________________________________________________________
////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * struct alloc_status - bit field, stores resource allocation flags
 * @dev_created: character device has been successfully created
 */
struct alloc_status {
	unsigned long dev_created : 1;
	unsigned long cdev_added : 1;
};
// start with everything non-done
static struct alloc_status alloc_flags = { 0 };

/**
 * struct hive_file_item - stores data for each descriptor
 * @buffer: memory we allocate for each file
 * @length: buffer size
 * @rdoffset: reader offset
 * @wroffset: writer offset
 */
struct hive_file_item {
	char *buffer;
	long length;
	long rdoffset;
	long wroffset;
};

/**
 * struct hive_flist_item - accounting of open descriptors
 * @list: fields to link the list
 * @file: created on open(), removed on close()
 *        changes during file operations, but ptr stays the same
 */
struct hive_flist_item {
	struct list_head list;
	struct file *file
};


LIST_HEAD(hive_flist);

char * magic_phrase = "Wow, we made these bees TWERK !\n";

static char *devname = THIS_MODULE->name;
module_param(devname, charp, 0);
MODULE_PARM_DESC(devname, "Name as in VFS. Defaults to module name\n");
static int major = 0;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major number. Defaults to 0 (automatic allocation\n)");
static int buffsize = 2 * sizeof(magic_phrase);
module_param(buffsize, int, 0);
MODULE_PARM_DESC(buffsize, "Char buffer size. Defaults to 2 * sizeof magic_phrase\n");

dev_t hive_dev = 0;	// Stores our device handle
static struct cdev hive_cdev; // scull-initialized

/**
 * hive_flist_new() - creates list item having buffer
 * @buffer_size: numer of characters in buffer
 */
static inline struct hive_flist_item *hive_flist_new(unsigned long buffer_size)
{
	struct hive_flist_item *item = kmalloc(sizeof (*item), GFP_KERNEL);
	if (NULL == item) {
		return NULL;
	}
	return item;
}

/**
 * hive_flist_rm() - deletes item from list and frees memory
 * @item: list item
 */
static inline void hive_flist_rm(struct hive_flist_item *item)
{
	if (NULL == item)
		return;
	list_del(&item->list);
	kfree(item);
}

/**
 * hive_flist_get - searches the list
 * @file: field of the list
 *
 * Return: item having the field or NULL if not found
 */
static struct hive_flist_item *hive_flist_get(struct file *file)
{
	struct hive_flist_item *item;
	list_for_each_entry(item, &hive_flist, list) {
		if (item->file == file)
			return item;
	}
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
	struct hive_file_item *item;

	char *buf = kmalloc(sizeof(*buf) * buffsize, GFP_KERNEL);
	if (NULL == buf) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		return -ENOMEM;
	}

	file->private_data = kmalloc(sizeof(struct hive_file_item), GFP_KERNEL);

	if (NULL == file->private_data) {
		MOD_DEBUG(KERN_ERR, "Memory allocation error for %p", file);
		kfree(buf);
		return -ENOMEM;
	}

	struct hive_flist_item *item_fl = hive_flist_new(0);
	if (NULL == item_fl) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		kfree(buf);
		kfree(file->private_data);
		return -ENOMEM;
	}


	// adding additional data to a file
	item = (struct hive_file_item *)file->private_data;
	item->buffer = buf;
	item->length = buffsize;
	item->rdoffset = 0;
	item->wroffset = 0;

	// adding a node to the list
	item_fl->file = file;
	list_add(&item_fl->list, &hive_flist);

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
	// delete additional data from a file
	struct hive_file_item *item;
	item = (struct hive_file_item *)file->private_data;
	kfree(item->buffer);	
	kfree(file->private_data);
	
	// remove a node from the list
	struct hive_flist_item *item_fl = hive_flist_get(file);
	if (NULL != item_fl)
		// remove item from list and free its memory
		hive_flist_rm(item_fl);

	MOD_DEBUG(KERN_DEBUG, "File entry %p unlinked", file);
	return 0;
}

/**
 * cdev_read() - called on file read() operation
 * @file: VFS file opened by a process
 * @buf: buffer obtained from userspace
 * @count: bytes requested to read in buffer
 * @loff: current position in file
 */
static ssize_t cdev_read(struct file *file, char __user *buf, 
			 size_t count, loff_t *loff)
{
	struct hive_file_item *item;

	item = (struct hive_file_item *)file->private_data;
	
	if (0 == item->rdoffset) {
		int count_temp = count < item->length ? count : item->length;
		int res = copy_to_user((void*)buf, item->buffer, count_temp);
		item->rdoffset = count_temp+1;
		put_user('\n', buf+count_temp);
		res = (int)count_temp+1;
		MOD_DEBUG(KERN_DEBUG, "Return bytes : %d", res);
		return res;
	}
	item->rdoffset = 0;
	MOD_DEBUG(KERN_DEBUG, "Return : EOF");
	return 0;
}

/**
 * cdev_write() - callback for file write() operation
 * @file: VFS file opened by a process
 * @buf: buffer obtained from userspace
 * @count: bytes requested to write from buffer
 * @loff: current position in file
 */
static ssize_t cdev_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *loff)
{
	int res;
	int len = count < buffsize ? count : buffsize;
	struct hive_file_item *item;
	item = (struct hive_file_item *)file->private_data;
	res = copy_from_user(item->buffer, (void*)buf, len);
	item->wroffset = len;
	
	if ('\n' == item->buffer[len-1])
		item->buffer[len-1] = '\0';
	else
		item->buffer[len] = '\0';
	MOD_DEBUG(KERN_DEBUG, "Put bytes : %d", res);
	return len;
}

/**
 * cdev_ioctl() - callback ioctl
 * (sends the magic phrase to the user)
 * @file: VFS file opened by a process
 * @cmd: team number (TYPE, NR, DIR, SIZE)
 * @arg: parameter from user space
*/
static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	if(_IOC_TYPE(cmd) != IOC_MAGIC) return -EINVAL;
	switch( cmd ) {
		case IOCTL_GET_STRING:
			if (copy_to_user((void*)arg, magic_phrase, _IOC_SIZE(cmd))) return -EFAULT;
			break;
		default: 
			return -ENOTTY;
	}
	return 0;
}

static struct file_operations hive_fops = {
	.open =           &cdev_open,
	.release =        &cdev_release,
	.read =           &cdev_read,
	.write =          &cdev_write,
	.unlocked_ioctl = &cdev_ioctl,
	// required to prevent module unloading while fops are in use
	.owner =          THIS_MODULE,
};


static void module_cleanup(void)
{
	// notice: deallocations happen in *reverse* order
	if (alloc_flags.cdev_added) {
		cdev_del(&hive_cdev);
	}
	if (alloc_flags.dev_created) {
		unregister_chrdev_region(hive_dev, 1);
	}

	// paranoid cleanup (afterwards to ensure all fops ended)
	struct hive_flist_item *item_fl;
	struct file *file;
	struct hive_file_item *item;

	list_for_each_entry(item_fl, &hive_flist, list) {
		file = item_fl->file;
		if (file->private_data != NULL ) {
			item = (struct hive_file_item *)file->private_data;
			kfree(item->buffer);	
			kfree(file->private_data);
		}
		hive_flist_rm(item_fl);
		MOD_DEBUG(KERN_DEBUG, "lst cleanup (should never happen)");
	}
     
     debugfs_remove_recursive(dirret);
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
	MOD_DEBUG(KERN_DEBUG, "This hive has %lu bees", 2 + jiffies % 8);
	// TODO: add stuff here to make module register itself in /dev

////////////////////////////////////////////////////////////////////////////////////////////////
//____________________________LAB7______________________________________________________________
////////////////////////////////////////////////////////////////////////////////////////////////
     
     struct dentry *junk;

     dirret = debugfs_create_dir("hive", NULL);
     if (!dirret) {
          MOD_DEBUG(KERN_ERR, "Diiret not create %d", err);
          return -ENOMEM;
     }

     junk = debugfs_create_file("debug", 0222, dirret, NULL, &fops_debug);     
     if (!junk) {
          MOD_DEBUG(KERN_ERR, "File not create %d", err);
          return -ENOMEM;
     }

     junk = debugfs_create_blob("test", 0777, dirret, blob_d);
     if (!junk) {
          MOD_DEBUG(KERN_ERR, "BLOB not create %d", err);
          return -ENOMEM;
     }
     
     blob_d = kmalloc(sizeof(struct debugfs_blob_wrapper), GFP_KERNEL);

     if (!blob_d) {
          MOD_DEBUG(KERN_ERR, "BLOB not mem %d", err);
          return -ENOMEM;
     }

     blob_d->data = (void *)magic_phrase;
     blob_d->size = buffsize / 2;
     
     junk = debugfs_create_u64("sum_test", 0777, dirret, &sum);
     if (!junk) {
          MOD_DEBUG(KERN_ERR, "sum not create %d", err);
          return -ENOMEM;
     }
////////////////////////////////////////////////////////////////////////////////////////////////
//____________________________LAB7______________________________________________________________
////////////////////////////////////////////////////////////////////////////////////////////////

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

