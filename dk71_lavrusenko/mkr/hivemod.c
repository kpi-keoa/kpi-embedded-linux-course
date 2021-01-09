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
#include <linux/device.h>

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("thodnev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

u64 beeval = 12;
struct dentry *ddir = NULL, *dfile = NULL;

#define MYDRBASE 'k'
#define SET_BUFF _IOW( MYDRBASE, 1, int*)
#define APPEND _IO( MYDRBASE, 2)

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
	unsigned long device_created :1;
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
static char *buff_ptr;
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
static struct class *cl; // Global variable for the device class
/**
 * hive_flist_new() - creates list item having buffer
 * @buffer_size: numer of characters in buffer
 */
static inline struct hive_flist_item *hive_flist_new(unsigned long buffer_size)
{
	// (!) here's where kernel memory (probably containing secrets) leaks
        // to userspace...
        // TODO: fix to make it zero'ed first
	char *buf = kmalloc(sizeof(*buf) * buffsize, GFP_KERNEL);
	if (NULL == buf)
		return NULL;

	struct hive_flist_item *item = kmalloc(sizeof *item, GFP_KERNEL);
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

//http://crossplatform.ru/?q=node/340 code parts take from
static ssize_t cdev_read(struct file *file, char __user *buf,
			 size_t count, loff_t *loff)
{
	struct hive_flist_item *item = hive_flist_get(file);
	if (NULL == item)
		return -EBADF;
	ssize_t bytes_read = 0;

	buff_ptr = item->buffer;
	if (*buff_ptr == 0)
    		return 0;

	while (count && *buff_ptr) {

  	put_user(*(buff_ptr++), buf++);

  	count--;
  	bytes_read++;
	}

	return bytes_read;
}

//http://crossplatform.ru/?q=node/340 code parts take from
static ssize_t cdev_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *loff)
{
	ssize_t i;
	struct hive_flist_item *item = hive_flist_get(file);
	if (NULL == item)
		return -EBADF;
	if (count > buffsize)
		count = buffsize;
		for (i = 0; i < count; i++) {
			if (0 != get_user((item->buffer)[i], buf)) {
				MOD_DEBUG(KERN_DEBUG, "Incorrect write ");
				return -1;
			}
			buf++;
	}
	return i;
}


// some parts take from http://www.cs.otago.ac.nz/cosc440/labs/lab06.pdf
static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {

	struct hive_flist_item *item = hive_flist_get(file);
	if (NULL == item)
		return -EBADF;
	if (_IOC_TYPE(cmd) != MYDRBASE)
		return -EINVAL;

	switch (cmd) {
	case APPEND:

		strcpy(item->buffer, magic_phrase);

		MOD_DEBUG(KERN_DEBUG, "Buff = %s\n", item->buffer);

		break;
	case SET_BUFF:

		item->length = arg;
		MOD_DEBUG(KERN_INFO, "Buff length is %li", item->length);
		break;
	default:
		return -ENOTTY;
	}
	return 0;
}


static struct file_operations hive_fops = {
	.open =    &cdev_open,
	.release = &cdev_release,
	.read =    &cdev_read,
	.write =   &cdev_write,
	.unlocked_ioctl = &cdev_ioctl,
	.owner =   THIS_MODULE,
};


static void module_cleanup(void)
{
	// notice: deallocations happen in *reverse* order
	if (alloc_flags.cdev_added) {
		cdev_del(&hive_cdev);
	}
	if (alloc_flags.device_created) {
		device_destroy(cl, hive_dev);
	}
	if (alloc_flags.class_created) {
		class_destroy(cl);
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

// some parts take from http://rus-linux.net/MyLDP/BOOKS/drivers/linux-device-drivers-05.html
static int __init cdevmod_init(void)
{
	int err = 0;
	ddir = debugfs_create_dir("hivemod", NULL);

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
	if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL) {
		goto err_handler;
	}
	alloc_flags.class_created = 1;
	if (device_create(cl, NULL, hive_dev, NULL, "hivedev") == NULL)
  	{
	goto err_handler;
	}
	alloc_flags.device_created = 1;
	cdev_init(&hive_cdev, &hive_fops);
	if ((err = cdev_add(&hive_cdev, hive_dev, 1))) {
		MOD_DEBUG(KERN_ERR, "Add cdev failed with %d", err);
		goto err_handler;
	}
	alloc_flags.cdev_added = 1;
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
