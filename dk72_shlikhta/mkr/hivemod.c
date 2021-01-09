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

/* Use 'k' as magic number */
#define IOC_MAGIC	'k'

#define SET_LENGTH 	_IOW(IOC_MAGIC, 0, int *)
#define APPEND_PHRASE 	_IO(IOC_MAGIC, 1)

#define IOC_MAXNR	1

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("thodnev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

u64 beeval = 12;
struct dentry *ddir = NULL;

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
 */
struct hive_item {
	char *buffer;
	unsigned long length;
	unsigned long rdoffset;
	unsigned long wroffset;
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
uint files_counter = 0;
//static struct class *hive_class = NULL;
//static struct device *hive_device = NULL;

/**
 * hive_item_init() - initialises struct for files
 * @buffer_size: numer of characters in buffer
 */
static inline struct hive_item *hive_item_init(unsigned long buffer_size)
{

	struct hive_item *tmp = kzalloc(sizeof(*tmp), GFP_KERNEL);
	char *tmp_buf = kzalloc(sizeof(*tmp_buf) * buffsize, GFP_KERNEL);

	if (NULL == tmp)
		return NULL;

	// (!) here's where kernel memory (probably containing secrets) leaks
        // to userspace...
	if (NULL == tmp_buf){
		kfree(tmp);	// avoid mem leaks
		return NULL;
	}

	tmp->buffer = tmp_buf;
	tmp->length = buffer_size;
	tmp->rdoffset = 0;
	tmp->wroffset = 0;
	return tmp;
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
	struct hive_item *item = hive_item_init(buffsize);
	if (NULL == item) {
		pr_err("Item or buffer allocate failed for %p\n", file);
		return -ENOMEM;
	}
	// fill the rest
	file->private_data = item;
	files_counter++;
	pr_info("New file entry %p created\n", file);

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
	files_counter--;
	pr_info("File close (%p)\n", file);

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

	struct hive_item *tmp_item = file->private_data;
	size_t buff_size = (size_t)tmp_item->length;
	unsigned long none_read = 0;

	unsigned long n = buff_size < count ? buff_size : count;

	none_read = copy_to_user(buf, tmp_item->buffer, n);

	if(none_read != 0) {
		pr_warn("Counldn't copy %lu bytes:\\ \n", none_read);
		return none_read;
	}

	return n;
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

	struct hive_item *tmp_item = file->private_data;
	size_t buff_size = (size_t)tmp_item->length;
	unsigned long none_read = 0;

	unsigned long n = buff_size < count ? buff_size : count;

	none_read = copy_to_user(tmp_item->buffer, buf, n);

	if(none_read != 0) {
		pr_warn("Counldn't copy %lu bytes:\\ \n", none_read);
		return none_read;
	}

	return n;
}

/**
 * cdev_lseek description()
 * @param  file   	VFS file opened by a process
 * @param  offset 
 * @param  orig   
 * 
 * @return        
 */
static loff_t cdev_lseek(struct file *file, loff_t offset, int orig)
{
	struct hive_item *tmp_item = file->private_data;
        loff_t new_pos = 0;
        printk(KERN_INFO "charDev : lseek function in work\n");
        switch (orig) {
        case 0 :        /*seek set*/
                new_pos = offset;
                break;
        case 1 :        /*seek cur*/
                new_pos = file->f_pos + offset;
                break;
        case 2 :        /*seek end*/
                new_pos = tmp_item->length - offset;
                break;
        default :
        	return -EINVAL;
        	break;

        }
        if (new_pos > tmp_item->length)
                new_pos = tmp_item->length;
        if (new_pos < 0)
                new_pos = 0;
        file->f_pos = new_pos;

        return new_pos;
}

/**
 * my_ioctl - callback ioctl
 * @file:        file pointer
 * @cmd:   	APPEND_PHRASE, SET_LENGTH
 * @arg:
 */
static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct hive_item *tmp_item = file->private_data;

	unsigned long buff_size = tmp_item->length;
	unsigned long buff_length = strlen(tmp_item->buffer);
	unsigned long length_of_magic_phrase = strlen(magic_phrase);

	if (_IOC_TYPE(cmd) != IOC_MAGIC) return -ENOTTY;
 	if (_IOC_NR(cmd) > IOC_MAXNR) return -ENOTTY;


	switch(cmd) {
	case APPEND_PHRASE:

		if((buff_size - buff_length) < length_of_magic_phrase) {
			buff_size = buff_length + length_of_magic_phrase;

			tmp_item->length = buff_size;

			if(NULL == krealloc(tmp_item->buffer, buff_size, GFP_KERNEL)) {
				pr_err("Counldn't reallocate memory  :(\n");
				return -ENOMEM;
			}
		}


		strlcat(tmp_item->buffer, magic_phrase, buff_size);
		pr_info("BUFFER = %s\n", tmp_item->buffer);

		break;
	case SET_LENGTH:

		tmp_item->length = arg;
		if(NULL == krealloc(tmp_item->buffer, arg, GFP_KERNEL)) {
				pr_err("Counldn't reallocate memory  :(\n");
				return -ENOMEM;
		}
		pr_info("New length of buffer is %lu\n", arg);

		break;
	default:
		return -ENOTTY;
	}

	return 0;
}

// This structure is partially initialized here
// and the rest is initialized by the kernel after call
// to cdev_init()
static struct file_operations hive_fops = {
	.open 		=    	&cdev_open,
	.release 	= 	&cdev_release,
	.read 		=    	&cdev_read,
	.write 		=   	&cdev_write,
	.unlocked_ioctl	= 	&cdev_ioctl,
	.llseek 	= 	&cdev_lseek,
	// required to prevent module unloading while fops are in use
	.owner 		=	THIS_MODULE,
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
	
	// device_destroy(hive_class, hive_dev);
	// class_destroy(hive_class);
	// paranoid checking (afterwards to ensure all fops ended)
	if(files_counter != 0) {
		pr_err("Some files still opened:(");	// should never happen
	}
}

static int __init cdevmod_init(void)
{
	int err = 0;
	ddir = debugfs_create_dir("hivemod", NULL);
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
	//alloc_flags.cdev_added = 1;
	MOD_DEBUG(KERN_DEBUG, "This hive has %lu bees", 2 + jiffies % 8);


	// TODO: add stuff here to make module register itself in /dev

	/* Create device classes */
	// hive_class = class_create(THIS_MODULE, "hive_class");
	// if (IS_ERR(hive_class)) {
	// 	err = PTR_ERR(hive_class);
	// 	goto err_handler;
	// } 

	/* Create device nodes */
	// hive_device = device_create(hive_class, NULL, hive_dev, NULL, "hive");
	// if (IS_ERR(hive_device)) {
	// 	err = PTR_ERR(hive_device);
	// 	goto err_handler;
	// }


	pr_info("Registered device with %d:%d\n", MAJOR(hive_dev), MINOR(hive_dev));

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
