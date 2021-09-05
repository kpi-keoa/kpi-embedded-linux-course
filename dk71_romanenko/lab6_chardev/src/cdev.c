#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>		// kmalloc()
#include <linux/uaccess.h>	// copy_to/from_user()
#include <linux/ioctl.h>

MODULE_DESCRIPTION("Char device ");
MODULE_AUTHOR("Cartman");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

#define WR_VALUE _IOW('a', 'a', int32_t *)
#define RD_VALUE _IOR('a', 'b', int32_t *)

int32_t value = 0;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev cdev_cdev;

static int __init cdevmod_init(void);
static void __exit cdevmod_exit(void);
static int cdev_open(struct inode *inode, struct file *file);
static int cdev_release(struct inode *inode, struct file *file);
static ssize_t cdev_read(struct file *filp, char __user *buf, size_t len,
			 loff_t *off);
static ssize_t cdev_write(struct file *filp, const char *buf, size_t len,
			  loff_t *off);
static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops = {
	.owner		= THIS_MODULE,
	.read		= cdev_read,
	.write		= cdev_write,
	.open		= cdev_open,
	.unlocked_ioctl	= cdev_ioctl,
	.release	= cdev_release,
};


static int cdev_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Device File Opened...!!!\n");
	return 0;
}


static int cdev_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Device File Closed...!!!\n");
	return 0;
}


static ssize_t cdev_read(struct file *filp, char __user *buf, size_t len,
			 loff_t *off)
{
	printk(KERN_INFO "Read Function\n");
	return 0;
}


static ssize_t cdev_write(struct file *filp, const char __user *buf, size_t len,
			  loff_t *off)
{
	printk(KERN_INFO "Write function\n");
	return 0;
}


static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case WR_VALUE:
		copy_from_user(&value, (int32_t *)arg, sizeof(value));
		printk(KERN_INFO "Value = %d\n", value);
		break;
	case RD_VALUE:
		copy_to_user((int32_t *)arg, &value, sizeof(value));
		break;
	}
	return 0;
}


static int __init cdevmod_init(void)
{
	if ((alloc_chrdev_region(&dev, 0, 1, "cdev_Dev")) < 0) {
		printk(KERN_INFO "Cannot allocate major number\n");
		return -1;
	}
	printk(KERN_INFO "Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

	cdev_init(&cdev_cdev, &fops);

	if ((cdev_add(&cdev_cdev, dev, 1)) < 0) {
		printk(KERN_INFO "Cannot add the device to the system\n");
		goto r_class;
	}

	if ((dev_class = class_create(THIS_MODULE, "cdev_class")) == NULL) {
		printk(KERN_INFO "Cannot create the struct class\n");
		goto r_class;
	}

	if ((device_create(dev_class, NULL, dev, NULL, "cdev_device")) ==
	    NULL) {
		printk(KERN_INFO "Cannot create the Device 1\n");
		goto r_device;
	}
	printk(KERN_INFO "Device Driver Insert...Done!!!\n");
	return 0;

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev, 1);
	return -1;
}


static void __exit cdevmod_exit(void)
{
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&cdev_cdev);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}


module_init(cdevmod_init);
module_exit(cdevmod_exit);
