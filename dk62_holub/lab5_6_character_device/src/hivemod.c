#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/fs.h>		// used by fs
#include <linux/rbtree.h>	// generic rb_tree header 
#include <linux/errno.h> 	// errno flags
#include <linux/slab.h>		// used by kernel alloc
#include <linux/ioctl.h>	// used by ioctl calls
#include <linux/cdev.h>		// used by char dev
#include <linux/uaccess.h>	// used by copy_to/from_user 

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("MaksHolub");
MODULE_VERSION("1.0");
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

#define CHG_BUF _IOW('V','a', unsigned long*)
#define ADD_PHR _IOW('B','b', unsigned long*)

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
struct rb_root mytree = RB_ROOT;

/**
 * struct hive_flist_item - stores data for each descriptor
 * @node: fields to link the tree
 * @file: created on open(), removed on close()
 *        changes during file operations, but ptr stays the same
 * @buffer: memory we allocate for each file
 * @length: buffer size
 * @rdoffset: reader offset
 * @wroffset: writer offset
 *
 * This implementation is not optimal as it imposes linear O(N)
 * lookup through list.
 * Completed: change this to tree 
 */ 
struct hive_item {
	struct rb_node node;
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
static struct class *hive_class = NULL;

/**
 * tree_insert() - insert item to tree
 * @root:      pointer to root 
 * @hive_item: item of struct hive_item
 */
int tree_insert(struct rb_root *root, struct hive_item *data)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	/* Figure out where to put new node */
	while (*new) {
		struct hive_item *this = container_of(*new, struct hive_item, node);
		int result = memcmp(data->file, this->file, sizeof(struct file));
		
		parent = *new;
		if (result < 0) {
			new = &((*new)->rb_left);
		} else if (result > 0) {

			new = &((*new)->rb_right);

		} else {
			return 1;
		}
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&data->node, parent, new);
	rb_insert_color(&data->node, root);

	return 0;
}

/**
 * hive_tree_new() - creates tree item having buffer
 * @buffer_size: numer of characters in buffer
 */
static inline struct hive_item *hive_tree_new(unsigned long buffer_size)
{
	// (!) here's where kernel memory (probably containing secrets) leaks
        // to userspace...
        // provided kzalloc to fix it
	char *buf = kzalloc(sizeof(*buf) * buffsize, GFP_KERNEL);
	if (NULL == buf)
		return NULL;
	struct hive_item *item = kzalloc(sizeof *item, GFP_KERNEL);
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
 * hive_tree_rm() - deletes item from tree and frees memory
 * @item: tree item
 */
static inline void hive_tree_rm(struct hive_item *item)
{
	if (NULL == item)
		return;
	rb_erase(&item->node, &mytree);
	kfree(item->buffer);
	kfree(item);
}

/**
 * hive_tree_get - searches the tree
 * @file: field of the tree
 *
 * Return: item having the field or NULL if not found
 */ 
static struct hive_item *hive_tree_get(struct rb_root *root, struct file *file)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct hive_item *data = container_of(node, struct hive_item, node);
		int result;

		result = memcmp(data->file, file, sizeof(struct file *));			

		if (result < 0) {
			node = node->rb_left; 
		} else if (result > 0) {
			node = node->rb_right;
		}
		else {
			return data;
		}
	}
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
	struct hive_item *item = hive_tree_new(buffsize);
	if (NULL == item) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		return -ENOMEM;
	}
	// fill the rest
	item->file = file;
	if (!tree_insert(&mytree, item)) {
		MOD_DEBUG(KERN_DEBUG, "New file entry %p created", file);
	} else {
		MOD_DEBUG(KERN_DEBUG, "New file entry not created");	
	}
	return 0;
}

/**
 * cdev_release() - file close() callback
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 */
static int cdev_release(struct inode *inode, struct file *file)
{
	struct hive_item *item = hive_tree_get(&mytree, file);
	if (NULL == item)
		return -EBADF;
	// remove item from tree and free its memory
	hive_tree_rm(item);
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
	struct hive_item *item = hive_tree_get(&mytree, file);
	if (NULL == item)
		return -EBADF;
	// Completed: Add buffer read logic.

	ssize_t retval = 0;

	if (*loff > item->length)		
		goto EXIT;
	if ((count + *loff) > item->length) {
		MOD_DEBUG(KERN_DEBUG, "Attempt to READ beyond the dev size!");
		/* read only upte the device size */
		count = item->length - *loff;
	}

	/* copy to user and update the offset in the device */
	retval = count - copy_to_user(buf, (item->buffer + *loff), count);
	*loff += retval;

	MOD_DEBUG(KERN_DEBUG, "%s: device: bytes read: %d, position: %d\n",\
			"hive", (int)retval, (int)*loff);
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
	struct hive_item *item = hive_tree_get(&mytree, file);
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

	MOD_DEBUG(KERN_DEBUG, "%s: bytes written: %d, position: %d\n",\
			"hive", (int)retval, (int)*loff);

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
 *		SEEK_END: set offset to end 
 * Return: new offset value
 */
static loff_t cdev_lseek(struct file *file, loff_t f_offset, int action)
{
	loff_t new_offset;
	struct hive_item *item = hive_tree_get(&mytree, file);
	if (NULL == item)
		return -EBADF;

	switch (action) {
		case SEEK_SET:
			new_offset = f_offset;
			break;
		case SEEK_END:
			new_offset= strlen(item->buffer);

		default:
			new_offset = -EINVAL;
			goto EXIT;
	}
	
	file->f_pos = new_offset;

	printk(KERN_DEBUG "%s: device: Seeking to position: %ld\n",\
		 "/dev/hive_dev", (long) new_offset);
EXIT:
	return new_offset;
}

/**
 * cdev_ioctl - callback ioctl (!test!)
 * @file:        file pointer
 * @ioctl_num:   CHG_BUF, ADD_PHR
 * @ioctl_param: perameter from userspace
 * Description:
 * 		CHG_BUF: set buffer size
 *		ADD_PHR: append magic phrase to buffer 
 * Return: 0 or -1 (if fail)
 */

long cdev_ioctl(struct file *file, unsigned int ioctl_num, 
			unsigned long ioctl_param)
{
	struct hive_item *item = hive_tree_get(&mytree, file);
	if (NULL == item)
		return -EBADF;
	switch(ioctl_num) {
                case CHG_BUF:
			if(ioctl_param > item->length) {
				char *buf = kzalloc(sizeof(*buf) * ioctl_param, 
							GFP_KERNEL);
				memcpy(buf, item->buffer, 
					sizeof(*buf)*item->length);
				kfree(item->buffer);
				item->buffer = buf;
				item->length = ioctl_param;
				
			} else {
				MOD_DEBUG(KERN_DEBUG, "Change buf not required");
				return -1;
			}                       
                        break;
                case ADD_PHR:
			if ((strlen(item->buffer) + buffsize/2) > item->length) {
				char *buf = kzalloc(sizeof(*buf) 
						* (item->length + buffsize/2), 
						GFP_KERNEL);				
				strcat(buf, item->buffer);
				kfree(item->buffer);
				strcat(buf, magic_phrase);
				item->buffer = buf;
				item->length = item->length + buffsize/2;
				
			} else {
				
				strcat(item->buffer, magic_phrase);
			}
			
                        break;
		default:
			return -1;
        }
        return 0;
}

// This structure is partially initialized here
// and the rest is initialized by the kernel after call
// to cdev_init()
// Completed: add ioctl to append magic phrase to buffer conents to
//       make these bees twerk
// Completed: add ioctl to select buffer size
static struct file_operations hive_fops = {
	.open =    &cdev_open,
	.release = &cdev_release,
	.read =    &cdev_read,
	.llseek =  &cdev_lseek,
	.write =   &cdev_write,
	.unlocked_ioctl = &cdev_ioctl,
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
	struct hive_item *item;
	struct rb_node *node;
	for (node = rb_first(&mytree); node; node = rb_next(node)) {
		item = rb_entry(node, struct hive_item, node);		
		hive_tree_rm(item);
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
