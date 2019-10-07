/**
 * based on code by thodnev
 */
#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>	// used by copy_to/from_user 

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("MaksHolub");
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
	{printk(level "%s: " fmt "\n", THIS_MODULE->name,##__VA_ARGS__);}
// How it works? What's inside THIS_MODULE?
// Are there any analogs?

#define  MODULE_CLASS_NAME  "hive_cdev_class"

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
	struct list_head list;
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
static struct class *hive_class = NULL;

/**
 * hive_flist_new() - creates list item having buffer
 * @buffer_size: numer of characters in buffer
 */
static inline struct hive_flist_item *hive_flist_new(unsigned long buffer_size)
{
	// (!) here's where kernel memory (probably containing secrets) leaks
        // to userspace...
        // provided kzalloc to fix it
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

/**
 * hive_flist_rm() - deletes item from list and frees memory
 * @item: list item
 */
static inline void hive_flist_rm(struct hive_flist_item *item)
{
	if (NULL == item)
		return;
	list_del(&item->list);
	kfree(item->buffer);
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
	struct hive_flist_item *item = hive_flist_new(buffsize);
	if (NULL == item) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		return -ENOMEM;
	}
	// fill the rest
	item->file = file;
	list_add(&item->list, &hive_flist);
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
	struct hive_flist_item *item = hive_flist_get(file);
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
 * @buf:   buffer provided from userspace
 * @count: bytes requested to read in buffer
 * @loff:  current position in the file
 */
static ssize_t cdev_read(struct file *file, char __user *buf, 
			 size_t count, loff_t *loff)
{
	struct hive_flist_item *item = hive_flist_get(file);
	if (NULL == item)
		return -EBADF;
	// Completed: Add buffer read logic.

	ssize_t retval = 0;

	if(*loff > item->length)		
		goto EXIT;
	if((count + *loff) > item->length) {
		MOD_DEBUG(KERN_DEBUG, "Attempt to READ beyond the dev size!");
		count = item->length - *loff;
	}

	/* copy to user and update the offset in the device */
	retval = count - copy_to_user(buf, (item->buffer + *loff), count);
	*loff += retval;

	MOD_DEBUG(KERN_DEBUG, " bytes read: %d, position: %d",\
			(int)retval, (int)*loff);
EXIT:
	return retval;	

	return 0;
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
	struct hive_flist_item *item = hive_flist_get(file);
	if (NULL == item)
		return -EBADF;

	//Completed: Add buffer write logic.
	ssize_t retval = -ENOMEM;

	if((count + *loff) > item->length) {
		MOD_DEBUG(KERN_DEBUG, "Attempt to write beyond the dev size!");
		goto EXIT;
	}

	/* copy to user and update the offset in the device */
	retval = count - copy_from_user((item->buffer + *loff), buf, count);
	*loff += retval;

	MOD_DEBUG(KERN_DEBUG, " bytes written: %d, position: %d",\
			 (int)retval, (int)*loff);

EXIT:
	return retval;
}

/**
 * cdev_lseek - callback lseek (!test!)
 * @file: file pointer
 * @f_offset: requested offset to be set the file
 * @action: SEEK_SET
 * Description:
 * 		SEEK_SET: set to requested offset
 *		...to be updated in future...
 * Return:
 */
static loff_t cdev_lseek(struct file *file, loff_t f_offset, int action)
{
	loff_t new_offset;
	struct hive_flist_item *item = hive_flist_get(file);
	if (NULL == item)
		return -EBADF;

	switch (action)
	{
		case SEEK_SET:
			new_offset = f_offset;
			break;

		default:
			new_offset = -EINVAL;
			goto EXIT;
	}
	
	file->f_pos = new_offset;

	MOD_DEBUG(KERN_DEBUG, "Seeking to position: %ld", (long)new_offset);
EXIT:
	return new_offset;
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
	.llseek =  &cdev_lseek,
	.write =   &cdev_write,
	// required to prevent module unloading while fops are in use
	.owner =   THIS_MODULE,
};


static void module_cleanup(void)
{
	// notice: deallocations happen in *reverse* order
	if (alloc_flags.cdev_added) {
		cdev_del(&hive_cdev);
		device_destroy(hive_class, hive_dev);
    		class_destroy(hive_class);
	}
	if (alloc_flags.dev_created) {
		unregister_chrdev_region(hive_dev, 1);
	}
	// paranoid cleanup (afterwards to ensure all fops ended)
	struct hive_flist_item *item;
	list_for_each_entry(item, &hive_flist, list) {
		hive_flist_rm(item);
		MOD_DEBUG(KERN_DEBUG, "lst cleanup (should never happen)");
	}
}

static int __init cdevmod_init(void)
{
	int err = 0;
	int retval = 0;

	
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
	// Completed: add stuff here to make module register itself in /dev
	if ((hive_class = class_create(THIS_MODULE, "hive_class")) == NULL) {
	    unregister_chrdev_region(hive_dev, 1);
	    return -1;
	}
	if (device_create(hive_class, NULL, hive_dev, NULL, "hive_dev") == NULL) {
		class_destroy(hive_class);
		unregister_chrdev_region(hive_dev, 1);
		return -1;
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
