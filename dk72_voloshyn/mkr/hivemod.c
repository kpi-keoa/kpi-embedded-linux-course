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
#include <linux/ioctl.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/kdev_t.h>

#define BEES_TWERK 0x410a01 // type 'A' with ordinal number
#define BUFF_SIZE 0x410a02

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("thodnev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

u64 beeval = 12;
struct dentry *ddir = NULL, *dfile = NULL;

int counter = 0;
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
};
// start with everything non-done
static struct alloc_status alloc_flags = { 0 };

/**
 * struct hive_fdata_item - stores data for each descriptor
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
struct hive_fdata_item {
	struct private_data data;
	struct file *file;
	char *buffer;
	long length;
	long rdoffset;
	long wroffset;
};

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
 * @buffer_size: numer of characters in buffer
 */
static inline struct hive_fdata_item *hive_fdata_new(unsigned long buffer_size)
{
	// (!) here's where kernel memory (probably containing secrets) leaks
        // to userspace...
        // TODO: fix to make it zero'ed first
	char *buf = kzalloc(sizeof(*buf) * buffsize, GFP_KERNEL);
	if (NULL == buf)
		return NULL;

	struct hive_fdata_item *item = kzalloc(sizeof *item, GFP_KERNEL);
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
	
	struct hive_fdata_item *item = hive_fdata_new(buffsize);
	if (NULL == item) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		return -ENOMEM;
	}
	// fill the rest
	file->private_data = item;
	counter++;
	
	MOD_DEBUG(KERN_DEBUG, "New file entry %p created, files count - %d", 
							file, (sizeof(files)/sizeof(files[0]));
	return 0;
}

/**
 * cdev_release() - file close() callback
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 */
static int cdev_release(struct inode *inode, struct file *file)
{
	kfree(file->private_data);

	if (NULL == item)
		return -EBADF;
	// remove item from list and free its memory
	counter--;
	free(files[counter]);
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
	struct hive_fdata_item* item = file->private_data;
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer read logic. Make sure seek operations work
    	//       correctly
	//       Be careful not to access array above bounds
	char* point_buffs = item->buffer;
	int count_bytes = 0;

	while (count && *point_buffs) {
		if (0 != put_user(*point_buffs, buff)) {
			++buf;
			return -1;
		}
		count--;
		count_bytes++;
		point_buffs++;
	}
	return count_bytes;
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
	struct hive_fdata_item *item = file->private_data;
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer write logic. Make sure seek operations work
    	//       correctly
	//       Be careful not to access array above bounds
	int i;
	if(count > buffsize)
		count = buffsize
		for (i = 0; i < count; i++) {
			if (0 != get_user((item->buffeer)[i], buf)) {
				MOD_DEBUG(KERN_DEBUG, "incorrect write ");
				return -1;
			}
			buf++;
	}
	return i;
}

static long cdev_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
	struct hive_fdata_item* item = file->private_data;
	if (NULL == item)
		return -EBADF;

	unsigned long size_buff = item->length;

	switch (cmd)
	{
	case BEES_TWERK:
		size_t just_size = strlcat(item->buffer, magic_phrase, size_buff);

		if (size < size_buff)
			MOD_DEBUG(KERN_DEBUG, "Our buffer can't contain TWERK our BEES =( ")
		else
			MOD_DEBUG(KERN_DEBUG, "%s Look at these BEES!!! Mama mia, that is TWERK!!!\n", magic_phrase);

		MOD_DEBUG(KERN_DEBUG, "And now, our buffer = %s\n", item->buffer);

		break;
	case BUFF_SIZE:
		item->lenght = arg;
		MOD_DEBUG(KERN_DEBUG, "New lenght = %lu\n", arg);

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
	.open			= &cdev_open,
	.release		= &cdev_release,
	.read			= &cdev_read,
	.write			= &cdev_write,
	.unlocked_ioctl = &cdev_ioctl,
	// required to prevent module unloading while fops are in use
	.owner			= THIS_MODULE,
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
	if (counter != 0)
		MOD_DEBUG(KERN_ERR, "Not all files are closed..");
}

static int __init cdevmod_init(void)
{
	int err = 0;
	ddir = debugfs_create_dir("hivemod", NULL);
	dfile = debugfs_create_u64("beeval", 0777, ddir, &beeval);
	
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

