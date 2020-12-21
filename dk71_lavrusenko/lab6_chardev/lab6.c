#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/debugfs.h>
#include <linux/device.h>

MODULE_DESCRIPTION("Character device");
MODULE_AUTHOR("Oleksandr Lavrusenko");
MODULE_VERSION("6.0");
MODULE_LICENSE("Dual MIT/GPL");


struct dentry *ddir = NULL, *dfile = NULL;


#define MOOD_STEALTH 0
#define MOOD_ONE 1
#define MOOD_TWO 2
#define MOOD_THREE 3

#define MYDRBASE 'k'
#define SET_MOOD	_IOW(MYDRBASE, 0, uint)


#define MOD_DEBUG(level, fmt, ...) \
	{printk(level "%s: " fmt "\n", THIS_MODULE->name,##__VA_ARGS__);}

struct alloc_status {
	unsigned long dev_created : 1;
	unsigned long cdev_added : 1;
};
// start with everything non-done
static struct alloc_status alloc_flags = { 0 };


static int device_open = 0;
static const char magic_phrase[] = "Oleksandr Lavrusenko dvice msg!";
static char *buff_ptr;
static char *devname = THIS_MODULE->name;

module_param(devname, charp, 0);
MODULE_PARM_DESC(devname, "Name as in VFS. Defaults to module name");

static int major = 0;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major number. Defaults to 0 (automatic allocation)");
static int buffsize = 2 * sizeof(magic_phrase);

static unsigned int dev_mood = 0;
module_param(dev_mood, uint, 0);
MODULE_PARM_DESC(dev_mood, "Device mood");

dev_t lab6_dev = 0;	// Stores our device handle
static struct cdev lab6_cdev; // scull-initialized



static int cdev_open(struct inode *inode, struct file *file)
{

	static int counter = 0;

        if (device_open)
                return -EBUSY;
	device_open++;
	switch (dev_mood) {
		case MOOD_STEALTH:
		sprintf(magic_phrase, "This device was opened %d times!\n", counter);
		break;
		case MOOD_ONE:
		sprintf(magic_phrase, "This device was opened %d times!\n", counter++);
		break;
		case MOOD_TWO:
		counter += MOOD_TWO;
		sprintf(magic_phrase, "This device was opened %d times!\n", counter);
		break;
		case MOOD_THREE:
		counter += MOOD_THREE;
		sprintf(magic_phrase, "This device was opened %d times!\n", counter);
		break;
		default:
		sprintf(magic_phrase, "This device was opened %d times!\n", counter++);
		break;
	}
        printk(KERN_ALERT "cdev_open was call for %d times\n", counter);
        buff_ptr = magic_phrase;
        try_module_get(THIS_MODULE);


	return 0;
}


static int cdev_release(struct inode *inode, struct file *file)
{
	device_open--;
	        /*
	         * Decrement the usage count, or else once you opened the file, you'll
	         * never get get rid of the module.
	         */
	module_put(THIS_MODULE);
	return 0;
}


static ssize_t cdev_read(struct file *file, char __user *buffer,
			 size_t count, loff_t *loff)
{
	/*
         * Number of bytes actually written to the buffer
         */
        int bytes_read = 0;

        /*
         * If we're at the end of the message,
         * return 0 signifying end of file
         */
        if (*buff_ptr == 0)
                return 0;

        /*
         * Actually put the data into the buffer
         */
        while (count && *buff_ptr) {

                /*
                 * The buffer is in the user data segment, not the kernel
                 * segment so "*" assignment won't work.  We have to use
                 * put_user which copies data from the kernel data segment to
                 * the user data segment.
                 */
                put_user(*(buff_ptr++), buffer++);

                count--;
                bytes_read++;
        }


        return bytes_read;
}


static ssize_t cdev_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *loff)
{
	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
        return -EINVAL;
}


static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {

	if (_IOC_TYPE(cmd) != MYDRBASE)
		return -EINVAL;

	switch(cmd) {
		case SET_MOOD:
			dev_mood = (uint)arg;
			pr_info("Now mood is - %d\n", (uint)arg);
			break;
		default:
			return -ENOTTY;
	}

		return 0;
}


static struct file_operations lab6_fops = {
	.open =    &cdev_open,
	.release = &cdev_release,
	.read =    &cdev_read,
	.write =   &cdev_write,
	.unlocked_ioctl = &cdev_ioctl,
	.owner =   THIS_MODULE,
};


static void module_cleanup(void)
{

	if (alloc_flags.cdev_added) {
		cdev_del(&lab6_cdev);
	}
	if (alloc_flags.dev_created) {
		unregister_chrdev_region(lab6_dev, 1);
	}

}


static int __init lab6_init(void)
{
	int err = 0;
	ddir = debugfs_create_dir("lab6", NULL);

	if (0 == major) {
		// use dynamic allocation (automatic)
		err = alloc_chrdev_region(&lab6_dev, 0, 1, devname);
	} else {
		// stick with what user provided
		lab6_dev = MKDEV(major, 0);
		err = register_chrdev_region(lab6_dev, 1, devname);
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
	          MAJOR(lab6_dev), MINOR(lab6_dev));
	cdev_init(&lab6_cdev, &lab6_fops);
	if ((err = cdev_add(&lab6_cdev, lab6_dev, 1))) {
		MOD_DEBUG(KERN_ERR, "Add cdev failed with %d", err);
		goto err_handler;
	}
	alloc_flags.cdev_added = 1;

	return 0;

err_handler:
	module_cleanup();
	return err;
}

static void __exit lab6_exit(void)
{
	module_cleanup();
	MOD_DEBUG(KERN_DEBUG, "LAB6 terminated");
	debugfs_remove_recursive(ddir);
}

module_init(lab6_init);
module_exit(lab6_exit);
