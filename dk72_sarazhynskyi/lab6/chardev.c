#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>

#define MOOD_DEFAULT 0
#define MOOD_INSANITY 1
#define MOOD_PLAYFUL 2
#define MOOD_AGGRESSIVE 3
#define MOOD_PHILOSOPHICAL 4

MODULE_DESCRIPTION("Char device to tell read count");
MODULE_AUTHOR("Sarazhynskyi Valentyn <valentins5135@gmail.com>");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

static int major = 0;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major number. Defaults to 0 (automatic allocation)");

static unsigned int mood = 0;
module_param(mood, uint, 0);
MODULE_PARM_DESC(mood, "Device mood");

static char *msg_Ptr;

struct pass_param {
	char msg[255];
};

#define IOC_MAGIC	'L'
#define SET_MOOD	_IOW(IOC_MAGIC, 0, uint)

static const char *devname = THIS_MODULE->name;

static uint files_counter = 0;
dev_t my_dev = 0;
static struct cdev my_cdev;

static int cdev_open(struct inode *inode, struct file *file)
{
	struct pass_param *params = kzalloc(sizeof(*params), GFP_KERNEL);
	if (NULL == params) {
		pr_err("Params allocate failed for %p\n", file);
		return -ENOMEM;
	}
	static int counter = 0;

	file->private_data = params;

	if (files_counter)
		return -EBUSY;

	files_counter++;
	switch (mood) {
		case MOOD_AGGRESSIVE:
		sprintf(params->msg, "You hear me, a piece of ***\n"
				"I see you here for the %d time!\n", counter++);
		break;
		case MOOD_INSANITY:
		sprintf(params->msg, "Did I ever tell you what insanity is?\n"
				"This is when you come here for the %d time\n"
				"and think that something will change\n", counter++);
		break;
		case MOOD_PLAYFUL:
		sprintf(params->msg, "Hmmm beautiful, are you back to me?\n"
				"You know this is the %d time?\n", counter++);
		break;
		case MOOD_PHILOSOPHICAL:
		sprintf(params->msg, "The foundation of wisdom is patience. %d\n", counter++);
		break;
		default:
		sprintf(params->msg, "Now it opened %d times!\n", counter++);
		break;
	}
	msg_Ptr = params->msg;
	try_module_get(THIS_MODULE);
	return 0;
}

static int cdev_release(struct inode *inode, struct file *file)
{
	kfree(file->private_data);
	files_counter--;
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t cdev_read(struct file *file, char __user *buf,
			 size_t count, loff_t *loff)
{
	int bytes_read = 0;
	if (*msg_Ptr == 0)
		return 0;

	while (count && *msg_Ptr) {
		put_user(*(msg_Ptr++), buf++);
		count--;
		bytes_read++;
	}
return bytes_read;
}

static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	if (_IOC_TYPE(cmd) != IOC_MAGIC)
		return -ENOTTY;

	switch(cmd) {
	case SET_MOOD:
		mood = (uint)arg;
		pr_info("Now mood is - %d\n", (uint)arg);
		break;
	default:
		return -ENOTTY;
	}

	return 0;
}


static struct file_operations pass_gen_fops = {
	.open 		=    	&cdev_open,
	.release 	= 	&cdev_release,
	.read 		=    	&cdev_read,
	.unlocked_ioctl	= 	&cdev_ioctl,
	// required to prevent module unloading while fops are in use
	.owner 		=	THIS_MODULE,
};

static int __init cdevmod_init(void)
{
	int status = 0;
	if (0 == major) {
		// use dynamic allocation (automatic)
		status = alloc_chrdev_region(&my_dev, 0, 1, devname);

	} else {
		// stick with what user provided
		my_dev = MKDEV(major, 0);
		status = register_chrdev_region(my_dev, 1, devname);
	}

	if (status) {
		goto err_handler;
	}

	cdev_init(&my_cdev, &pass_gen_fops);
	// after call below the device becomes active
	// so all stuff should be initialized before
	if ((status = cdev_add(&my_cdev, my_dev, 1))) {
		goto err_handler;
	}
	pr_info("Registered device with %d:%d\n", MAJOR(my_dev), MINOR(my_dev));

	return 0;

err_handler:
	return status;
}

static void __exit cdevmod_exit(void)
{
	cdev_del(&my_cdev);
	unregister_chrdev_region(my_dev, 1);

	// paranoid checking (afterwards to ensure all fops ended)
	if(files_counter != 0) {
		pr_err("Some files still opened:(");	// should never happen
	}

}

module_init(cdevmod_init);
module_exit(cdevmod_exit);
