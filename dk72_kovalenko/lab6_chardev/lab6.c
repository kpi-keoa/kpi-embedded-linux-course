/* Some code is borrowed from Alex Shlikhta */

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
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/random.h>

MODULE_DESCRIPTION("Character device password generator");
MODULE_AUTHOR("sankodk72");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

static int major = 0;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major number. Defaults to 0 (automatic allocation)");

static int pass_length = 5;
module_param(pass_length, uint, 0);
MODULE_PARM_DESC(pass_length, "Lenth of generated password");

static bool spec_sym_en = false;
module_param(spec_sym_en, bool, 0);
MODULE_PARM_DESC(spec_sym_en, "Enable special symbols in password");

struct pass_param {
	uint length;
	bool spec_sym;
};

/* Use 'k' as magic number */
#define IOC_MAGIC	'k'

#define SET_LENGTH	_IOW(IOC_MAGIC, 0, uint)
#define SPEC_SYM_STAT	_IOW(IOC_MAGIC, 1, bool)

#define IOC_MAXNR	1

#define NUMBER		0
#define SMALL_LETTER	1
#define BIG_LETTER	2
#define SPEC_SYM 	3


static const char SPEC_SYM_ARRAY[] = { 
	'!',
	'"',
	'#',
	'$',
	'%',
	'&',
	'\'',
	'(',
	')',
	'*',
	'+',
	',',
	'-',
	'.',
	'/',
	':', 
	';', 
	'<', 
	'=', 
	'>', 
	'?', 
	'@', 
	'[', 
	'\\', 
	']', 
	'^', 
	'_', 
	'`', 
	'{', 
	'|', 
	'}', 
	'~'
};

static const char *devname = THIS_MODULE->name;

static uint files_counter = 0;
dev_t my_dev = 0;	// Stores our device handle
static struct cdev my_cdev; // scull-initialized

static int get_rand_range_int(const int min, const int max) 
{
	uint tmp;
	get_random_bytes(&tmp, 3);
	return tmp % (max - min + 1) + min;
}

static char *pass_gen(struct pass_param params)
{
	char *pass_buff = kzalloc(sizeof(*pass_buff) * params.length, GFP_KERNEL);

	uint i = 0;
	for(i = 0; i < params.length; i++) {
		int sym_type = get_rand_range_int(0, params.spec_sym ? 3 : 2);

		switch(sym_type) {

		case SMALL_LETTER:
			pass_buff[i] = get_rand_range_int('a', 'z');
			break;

		case BIG_LETTER:
			pass_buff[i] = get_rand_range_int('A', 'Z');
			break;

		case SPEC_SYM:
			pass_buff[i] = SPEC_SYM_ARRAY[get_rand_range_int(0, sizeof(SPEC_SYM_ARRAY) - 1)];
			break;
		default:
		case NUMBER:
			pass_buff[i] = get_rand_range_int('0', '9');
			break;

		}
	}

	return pass_buff;
}


/**
 * cdev_open() - callback for open() file operation
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 *
 * 
 */
static int cdev_open(struct inode *inode, struct file *file)
{
	struct pass_param *params = kzalloc(sizeof(*params), GFP_KERNEL);
	if (NULL == params) {
		pr_err("Params allocate failed for %p\n", file);
		return -ENOMEM;
	}
	
	params->length = pass_length;
	params->spec_sym = spec_sym_en;

	file->private_data = params;
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

	struct pass_param *params = file->private_data;
	unsigned long none_read = 0;

	unsigned long n = params->length < count ? params->length : count;

	char *password = pass_gen(*params);
	none_read = copy_to_user(buf, password, n);

	if(none_read != 0) {
		pr_warn("Counldn't read %lu bytes of password:\\ \n", none_read);
		return none_read;
	}

	return n;
}

/**
 * my_ioctl - callback ioctl
 * @file:        file pointer
 * @cmd:   	SET_LENGTH, SPEC_SYM_STAT
 * @arg:
 */
static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct pass_param *params = file->private_data;

	if (_IOC_TYPE(cmd) != IOC_MAGIC) return -ENOTTY;
 	if (_IOC_NR(cmd) > IOC_MAXNR) return -ENOTTY;

	switch(cmd) {
	case SET_LENGTH:

		params->length = (uint)arg;
		pr_info("New length of password is %d\n", (uint)arg);

		break;
	case SPEC_SYM_STAT:

		params->spec_sym = (bool)arg;
		pr_info("Special symbols are enabled\n");	

		break;
	default:
		return -ENOTTY;
	}

	return 0;
}


// This structure is partially initialized here
// and the rest is initialized by the kernel after call
// to cdev_init()
static struct file_operations pass_gen_fops = {
	.open 		=    	&cdev_open,
	.release 	= 	&cdev_release,
	.read 		=    	&cdev_read,
	.unlocked_ioctl	= 	&cdev_ioctl,
	// required to prevent module unloading while fops are in use
	.owner 		=	THIS_MODULE,
};

static int __init lab6_init(void)
{
	int status = 0;

	if(pass_length <= 0) {
		pr_err("Can't create password with length: %d:(\n", pass_length);
		status = -EINVAL;
		goto err_handler;
	}

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
	pr_info("Length of password is %d\n", pass_length);

	if(spec_sym_en)
		pr_info("Spec symbols is enabled\n");
	else
		pr_info("Spec symbols is disabled\n");
	return 0;

err_handler:
	return status;
}

static void __exit lab6_exit(void)
{
	cdev_del(&my_cdev);
	unregister_chrdev_region(my_dev, 1);
	
	// paranoid checking (afterwards to ensure all fops ended)
	if(files_counter != 0) {
		pr_err("Some files still opened:(");	// should never happen
	}	
	
}
 
module_init(lab6_init);
module_exit(lab6_exit);

