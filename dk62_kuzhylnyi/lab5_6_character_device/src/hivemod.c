/* 
 * The basic code has been borrowed from master branch
 * Author: "thodnev"
*/
#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h> 
#include <linux/kdev_t.h>
#include <linux/uaccess.h>	// used by copy_to|from_user ops
#include <asm/ioctl.h>
#include <linux/hashtable.h>

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("jayawar");
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


/**
 * struct alloc_status - bit field, stores resource allocation flags
 * @dev_created: character device has been successfully created
 */
struct alloc_status {
	unsigned long dev_created      : 1;
	unsigned long cdev_added       : 1;
        unsigned long dev_registered   : 1;
        unsigned long class_created    : 1;

};
// start with everything non-done
static struct alloc_status alloc_flags = { 0 };

/**
 * struct hive_hlist_item - stores data for each descriptor
 * @list: fields to link the hash table or associative array
 * @buffer: memory we allocate for each file
 * @length: buffer size
 *
 * This implementation provides O(1) complexity and in addition 
 * unnecessary keep file *file in here. 
 * The *file atc as key to each descriptor. 
 * As extention is useful to calculate a hash from *file. 
 */
struct hive_hlist_item {
        struct hlist_node node;
	char *buffer;
	long length;
};


DEFINE_HASHTABLE(hive_hlist, 3);

static const char magic_phrase[] = "Wow, we made these bees TWERK !";

static char *devname = THIS_MODULE->name;
module_param(devname, charp, 0);
MODULE_PARM_DESC(devname, "Name as in VFS. Defaults to module name");
static int major = 0;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major number. Defaults to 0 (automatic allocation)");
static int buffsize = 10 * sizeof(magic_phrase);
module_param(buffsize, int, 0);
MODULE_PARM_DESC(buffsize, "Char buffer size. Defaults to 2 * sizeof magic_phrase");

dev_t hive_dev = 0;	// Stores our device handle
static struct cdev hive_cdev; // scull-initialized
static struct class *char_class = NULL; ///< The device-driver class struct pointer
static struct device *char_device = NULL; ///< The device-driver device struct pointer


/**
 * hive_hlist_new() - creates list item having buffer
 * @buffer_size: numer of characters in buffer
 * 
 */
static inline struct hive_hlist_item *hive_hlist_new(unsigned long buffer_size)
{
        /*
        * The vmalloc() function works in a similar fashion to kmalloc(), 
        * except it allocates memory that is only virtually contiguous and not necessarily physically contiguous.
        * It is the most high effiency approach to operate with buffers.
        */
        char *buf = vmalloc(sizeof(*buf) * buffsize);
	if (NULL == buf)
		return NULL;

	struct hive_hlist_item *item = vmalloc(sizeof (*item));
	if (NULL == item) {
		kfree(buf);	// avoid mem leaks
		return NULL;
	}

	item->buffer = buf;
	item->length = buffer_size;
	return item;
}

/**
 * hive_hlist_rm() - deletes item from list and frees memory
 * @item: list item
 */
static inline void hive_hlist_rm(struct hive_hlist_item *item)
{
	if (NULL == item)
		return;
	hash_del(&item->node);
	vfree(item);
	vfree(item->buffer);
}

/**
 * hive_hlist_get - searches the list
 * @file: key to search entry 
 *
 * Return: item having the field or NULL if not found
 */
static struct hive_hlist_item *hive_hlist_get(struct file *file)
{
	struct hive_hlist_item *item;
        hash_for_each_possible(hive_hlist, item, node, (size_t *)file)
			return item;
	
	// not found
	MOD_DEBUG(KERN_ERR, "Expected list entry not found %p", file);
	return NULL;
}

/**
 * cdev_open() - callback for open() file operation
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 *
 * Allocates memory, creates fd entry and adds it to hash table
 */
static int cdev_open(struct inode *inode, struct file *file)
{
	struct hive_hlist_item *item = hive_hlist_new(buffsize);
	if (NULL == item) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		return -ENOMEM;
	}
	// fill the rest
        hash_add(hive_hlist, &item->node, (size_t *)file);
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
	struct hive_hlist_item *item = hive_hlist_get(file);
	if (NULL == item)
		return -EBADF;
	// remove item from list and free its memory
	hive_hlist_rm(item);
	MOD_DEBUG(KERN_DEBUG, "File entry %p unlinked", file);
	return 0;
}

/**
 * cdev_lseek() - called on file read() operation
 * @file: VFS file opened by a process
 * @ioctl_cmd: lseek flag
 * @arg: user argument. here should be new buffer size 
 * 
 * This lseek uses only SEEK_SET SEEK_END seeking flags
 */
static loff_t cdev_lseek(struct file *file, loff_t f_offset, int flag)
{
	loff_t seek;
        
	struct hive_hlist_item *item = hive_hlist_get(file);
	if (NULL == item)
		return -EBADF;
        
	switch (flag) {
		case SEEK_SET:
			seek = (f_offset < 0) ? 0 : f_offset;
                break;
		case SEEK_END:
			seek = item->length;
                break;
		default:
                        return -EINVAL;
                break;
	}

        /* teleport to a new position in the file */
	file->f_pos = seek;
        
	printk(KERN_DEBUG "%s Scull: Got new seek: %ld\n",\
              "/dev/hive_dev", (unsigned long) seek);
        
	return seek;
}

#define BUFFER_REALLOC _IOW('C', 1, unsigned long*)
#define ADD_MAGIC_PHRASE _IO('H', 2)

/**
 * cdev_ioctl() - called on file ioctl() operation
 * @file: VFS file opened by a process
 * @ioctl_cmd: available operations
 * @arg: user argument. here should be new buffer size 
 */
static long cdev_ioctl(struct file *file, unsigned int ioctl_cmd, unsigned long arg)
{
	struct hive_hlist_item *item = hive_hlist_get(file);
	if (NULL == item)
                return -EBADF;
        
        unsigned long new_buffer_size = 0;
        switch (ioctl_cmd) {
                case BUFFER_REALLOC:
                        new_buffer_size = arg;
                        
                        if ( NULL == item->buffer ) 
                                break;
                        /* Yes, this ioctl filed is able to reduce buffer only */
                        if (new_buffer_size < 0) 
                                break;
                                                
                        char *temp = vmalloc(sizeof(*temp) * new_buffer_size);
                        if (NULL == temp) {
                                printk(KERN_DEBUG "Cannot allocate temp char buffer allocation \n");
                                return -EBADF;
                        }
                        strcpy(temp, item->buffer);
                  
                        vfree(item->buffer);

                        item->buffer = vmalloc(sizeof(*item->buffer) * new_buffer_size);
                        memcpy(item->buffer, temp, new_buffer_size);
                       
                        vfree(temp);

                        item->length = new_buffer_size;
                        file->f_pos += new_buffer_size;
                        
                break;
                case ADD_MAGIC_PHRASE:
                        if ( NULL == item->buffer ) 
                                break;
                        
                        new_buffer_size = item->length + sizeof(magic_phrase);
                               
                        char *tmpbuff = vmalloc(sizeof(*tmpbuff) * new_buffer_size);
                        if (NULL == tmpbuff) {
                                printk(KERN_DEBUG "Cannot allocate temp char buffer \n");
                                return -EBADF;
                        }
                        memcpy(tmpbuff, item->buffer, item->length);
                        vfree(item->buffer);
                        
                        /* append phrase to an increased buffer */
                        strncat(tmpbuff, magic_phrase, new_buffer_size);
                        
                        if( NULL == (item->buffer = vmalloc(new_buffer_size) )) {
                                printk(KERN_DEBUG "Cannot reallocate a new buffer \n");
                                return -EBADF;
                        }
                        
                        memcpy(item->buffer, tmpbuff, new_buffer_size);
                        
                        item->length += sizeof(magic_phrase);
                        file->f_pos += sizeof(magic_phrase);
                        printk(KERN_DEBUG, "Magic phrase has been appended \n");
                break;
                default:
                        return -ENOTTY;
                break;
        }
        return 0;
}

/**
 * cdev_read() - called on file read() operation
 * @file: VFS file opened by a process
 * @buf: 
 * @size:
 * @loff:
 */
static ssize_t cdev_read(struct file *file, char __user *buf, 
			 size_t size, loff_t *loff)
{
        ssize_t ret_status = 0;
        
	struct hive_hlist_item *item = hive_hlist_get(file);
	if (NULL == item) {
		ret_status = -EBADF;
                goto rd_error; 
        }
                
        if ( item->length <=  *loff ) {
                ret_status = 0;
                printk(KERN_DEBUG "Already saved: %lld\n", *loff);
                goto rd_error;
        }
                
        /* determine minimal buffer length to operate*/
        size_t len = min(item->length - (size_t)*loff, size);
        printk(KERN_DEBUG "Scull read buffer length: %ld\n", len);

        if (len <= 0) {
                ret_status = -EBADF;
                printk(KERN_DEBUG "Scull buffer is empty or queried offset if too small: %ld\n", ret_status);
                goto rd_error;
        }
        
        /* Copy a block of data into user space with auto success read size checking*/
        if ( copy_to_user(buf, item->buffer + *loff, len) ) {
                printk(KERN_DEBUG "Read operation has been failed: %ld\n", ret_status);
                ret_status = -EFAULT;
                goto rd_error;
        }

        ret_status = len;
        *loff += ret_status;  
        
        MOD_DEBUG(KERN_INFO, "hive-scull has read %ld bytes\n", (ssize_t)ret_status);
        
        /* return size of read bytes or 0 if no error happened */
	return ret_status;
        
        rd_error:
                return ret_status;
}

/**
 * cdev_write() - callback for file write() operation
 * @file: VFS file opened by a process
 * @buf:
 * @size:

 * @loff:
 */
static ssize_t cdev_write(struct file *file, const char __user *buf,
			  size_t size, loff_t *loff)
{
        
        ssize_t ret_status = 0;

	struct hive_hlist_item *item = hive_hlist_get(file);
	if (NULL == item)
		return -EBADF;

        printk(KERN_DEBUG "Scull write buffer length: 1)%ld 2)%ld\n", item->length, size);

        if ( item->length <=  *loff ) {
                ret_status = 0;
                printk(KERN_DEBUG "Already saved: %lld\n", *loff);
                goto wr_error;
        }
        
        if ( (item->length - (size_t)*loff ) < size) {
                ret_status = -ENOSPC;
                printk(KERN_DEBUG "Already saved: %lld\n", *loff);
                goto wr_error;
        }
        
        if (copy_from_user(item->buffer + (size_t)*loff, buf, size)) {
                ret_status = -EFAULT;
                printk(KERN_DEBUG "User buffer is empty or queried offset if too small: %ld\n", ret_status);
                goto wr_error;
        }
        
        ret_status = size;
        *loff += ret_status;       

        printk(KERN_DEBUG "Scull wrote %ld bytes\n", ret_status);

        return ret_status;

        wr_error:
                return ret_status;
}

static struct file_operations hive_fops = {
	.owner =   THIS_MODULE,
	.llseek =  cdev_lseek,
	.open =    cdev_open,
	.release = cdev_release,
	.read =    cdev_read,
	.write =   cdev_write,
        .unlocked_ioctl = cdev_ioctl         
};


/**
 * module_cleanup() - smart cleaner function
 * Clean only busy resources defined via bite fields
 */
static void module_cleanup(void)
{
	if (alloc_flags.dev_registered) {
                device_destroy(char_class, hive_dev);
        }
        if (alloc_flags.class_created) {
                class_unregister(char_class);
        }
        if (alloc_flags.cdev_added) {
		cdev_del(&hive_cdev);
	}
	if (alloc_flags.dev_created) {
		unregister_chrdev_region(hive_dev, 1);
	}
	
	class_destroy(char_class);
}

static int __init cdevmod_init(void)
{
	int err = 0;
	
	if (major) {
                // stick with what user provided
                hive_dev = MKDEV(major, 0);
                err = register_chrdev_region(hive_dev, 1, devname);
	} else {
                // use dynamic allocation (automatic)
		err = alloc_chrdev_region(&hive_dev, 0, 1, devname);
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
        
        if (NULL == (char_class = class_create(THIS_MODULE, devname))) {               
                MOD_DEBUG(KERN_DEBUG, "Failed to register device class\n");
                goto err_handler;
        }
        alloc_flags.class_created = 1;
        
        char_device = device_create(char_class, NULL, hive_dev, NULL, devname);
        if (NULL == char_device) {
                MOD_DEBUG(KERN_DEBUG, "Failed to create the device\n");
                goto err_handler;
        }
        alloc_flags.dev_registered = 1;
        
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
