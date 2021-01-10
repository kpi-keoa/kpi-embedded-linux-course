#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/kdev_t.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/slab.h>          
#include<linux/uaccess.h>      
#include<linux/ioctl.h>

MODULE_DESCRIPTION("Calculator chardev");
MODULE_AUTHOR("Sad");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)


int32_t value1 = 0 ;
int32_t value2 = 0 ;
int32_t value = 0 ;
int32_t oper = 0 ;

dev_t dev = 0;

static struct class *dev_class;
static struct cdev sad_cdev ;

static long sad_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	static int count = 0;
	switch(cmd) {

		case WR_VALUE:
		if (count == 0)
		{
			copy_from_user(&oper, (int32_t*)arg, sizeof(oper));
			printk(KERN_INFO "oper = %d\n",oper);
			break;
		}
		else if (count == 1){
			copy_from_user(&value1, (int32_t*)arg, sizeof(value1));
			printk(KERN_INFO "value1 = %d\n",value1);
			break;
		}
		else if (count == 2){
			copy_from_user(&value2, (int32_t*)arg, sizeof(value2));
			printk(KERN_INFO "value2 = %d\n",value2);
			break;
		}
		case RD_VALUE:
		if (oper == 1)
			value = value1 + value2 ;
		else if (oper == 2)
			value = value1 - value2;
		else if (oper == 3)
			value = value1 * value2;
		else if (oper == 4)
			value = value1 / value2;
		else
			break;
		copy_to_user((int32_t*) arg, &value,sizeof(value));
		break;
	}

	count++; 
	if(count == 3)
	count = 0 ;

	return 0;
}

static struct file_operations fops = 
{
	.owner      	= THIS_MODULE,
	.unlocked_ioctl = sad_ioctl,
};

static int __init sad_driver_init(void)
{
	if ((alloc_chrdev_region(&dev,0,1,"sad_dev")) < 0){
		printk(KERN_INFO "cannot allocate major number\n");
		return -1;
	}

	printk(KERN_INFO " MAJOR = %d Minor = %d\n",MAJOR(dev),MINOR(dev));

	cdev_init(&sad_cdev,&fops);

	if ((cdev_add(&sad_cdev,dev,1)) < 0){
		printk(KERN_INFO "cannot add device to the system\n");
		goto r_class;
        }
     	
	/*Creating struct class*/
	(dev_class = class_create(THIS_MODULE,"sad_class")
	if (dev_class == NULL){
		printk(KERN_INFO "Cannot create the struct class\n");
		goto r_class;
	}

        /*Creating device*/
	if ((device_create(dev_class,NULL,dev,NULL,"sad_device")) == NULL){
		printk(KERN_INFO "Cannot create the Device 1\n");
		goto r_device;
	}

	printk(KERN_INFO "Device Driver Insert...Done!!!\n");
	return 0;

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;
}

void __exit sad_driver_exit(void)
{
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&sad_cdev);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "i've done...\n");
}

module_init(sad_driver_init);
module_exit(sad_driver_exit);

