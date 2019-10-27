/* 
* Based on @thodnev hivemod.c
* The information was taken from:
* - LKD 3ed.
* - RBT kernel API - https://www.infradead.org/~mchehab/kernel_docs/unsorted/rbtree.html
* - Character driver example - https://gist.github.com/ksvbka/0cd1a31143c1003ce6c7
* - Max's Holub hivemod.c - https://bit.ly/2o3rmvl
*/


#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/fs.h>
#include <linux/rbtree.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>	// used by ioctl calls
#include <linux/cdev.h>		// used by char dev
#include <linux/uaccess.h>	// used by copy_to/from_user 

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("rtchoke");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

#define UPD_BUFFER _IOW('D', 'i', unsigned long *)
#define ADD_MGC_PHR _IOW('C', 'k', unsigned long *)
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
 * struct hive_item - stores data for each descriptor
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
struct hive_item {
	struct rb_node node;
	struct file *file;
	char *buffer;
	long length;
	long rdoffset;
	long wroffset;
};

struct rb_root hive_tree = RB_ROOT;
static struct class *hive_class = NULL;
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

static int hive_tree_add_item(struct rb_root *root, struct hive_item *data)
{
       struct rb_node **new_item = &(root->rb_node), *parent = NULL;

       while(*new_item) {
               struct hive_item *this = container_of(*new_item, struct hive_item, node);
               int result = memcmp(data->file, this->file, sizeof(struct file ));
               parent = *new_item;
               if(result < 0) {
                       new_item = &((*new_item)->rb_left);
               } else if (result > 0) {
                       new_item = &((*new_item)->rb_right);
               } 
               else {
                       return 1;
               }

       } 
       rb_link_node(&data->node, parent, new_item);
       rb_insert_color(&data->node, root);
       return 0;
}

static void hive_tree_rm_item(struct hive_item *item)
{
        if(NULL == item) {
                return;
        }
        rb_erase(&item->node, &hive_tree);
        kfree(item->buffer);
        kfree(item);
}

static struct hive_item *hive_tree_get_item(struct rb_root *root, struct file *file)
{
        struct rb_node *node = root->rb_node;
        
        while(node) {
                struct hive_item *item = container_of(node, struct hive_item, node);
                int res;

                res = memcmp(item->file, file, sizeof(struct file *));
                if(res < 0) {
                       node = node->rb_left;
                } else if (res > 0) {
                       node = node->rb_right;
                } 
                else {
                       return item;
                }

        }
        return NULL;
}


static inline struct hive_item *hive_tree_new(unsigned long buffer_size)
{
	// (!) here's where kernel memory (probably containing secrets) leaks
        // to userspace...
        // TODO: fix to make it zero'ed first
        char *buf = NULL;
	buf = kmalloc(sizeof(*buf) * buffsize, GFP_KERNEL);
	if (NULL == buf)
		return NULL;

	struct hive_item *item = kmalloc(sizeof *item, GFP_KERNEL);
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
 * Allocates memory, creates fd entry and adds it to the rbt
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
	if(!hive_tree_add_item(&hive_tree, item)) {
                MOD_DEBUG(KERN_DEBUG, "New file entry %p created", file);
        } else {
                MOD_DEBUG(KERN_DEBUG, "Can't create file entry");
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
	struct hive_item *item = hive_tree_get_item(&hive_tree, file);
	if (NULL == item)
		return -EBADF;
	// remove item from tree and free its memory
	hive_tree_rm_item(item);
	MOD_DEBUG(KERN_DEBUG, "File entry %p unlinked", file);
	return 0;
}

/**
 * cdev_read() - called on file read() operation
 * @file: VFS file opened by a process
 * @buf: buffer where read from
 * @count: how many bytes should be read
 * @loff: current position in the file
 */
static ssize_t cdev_read(struct file *file, char __user *buf, 
			 size_t count, loff_t *loff)
{
	struct hive_item *item = hive_tree_get_item(&hive_tree, file);
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer read logic. Make sure seek operations work
    	//       correctly
	//       Be careful not to access array above bounds

        ssize_t result = 0;
        loff_t curr_pos = *loff;
        
        if(curr_pos > item->length)
                return result;
        if((curr_pos + count) > item->length) {
                MOD_DEBUG(KERN_DEBUG, "READ beyond bounds!");
                count = item->length - curr_pos;
        }

        result = count - copy_to_user(buf, curr_pos + item->buffer, count);
        *loff += result;

        MOD_DEBUG(KERN_DEBUG, "read %d bytes from position: %d\n", \
                                (int)result, (int)curr_pos);

	return result;
}

/**
 * cdev_write() - callback for file write() operation
 * @file: VFS file opened by a process
 * @buf: buffer where to write
 * @count: how many bytes should be written
 * @loff: current position in the flie
 */
static ssize_t cdev_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *loff)
{
	struct hive_item *item = hive_tree_get_item(&hive_tree, file);
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer write logic. Make sure seek operations work
    	//       correctly
	//       Be careful not to access array above bounds
        ssize_t result = -ENOMEM;
        loff_t curr_pos = *loff;

       if((curr_pos + count) > item->length) {
                MOD_DEBUG(KERN_DEBUG, "WRITE beyond bounds!");
                return result;
        }

        result = count - copy_from_user(curr_pos + item->buffer, buf, count);
        *loff += result;

        MOD_DEBUG(KERN_DEBUG, "write %d bytes to position: %d\n", \
                                (int)result, (int)curr_pos);
	return result;
}

static loff_t cdev_lseek(struct file *file, loff_t offset, int option)
{
        loff_t new_position = 0;
        struct hive_item *item = hive_tree_get_item(&hive_tree, file);
        if (NULL == item)
		return -EBADF;

        switch (option) {
        case SEEK_SET:
                new_position = offset;
                break;
        case SEEK_CUR:
                new_position = file->f_pos + offset;
                break;
        case SEEK_END:
                offset = sizeof(item->buffer) / sizeof(item->buffer[0]);
                break;
        default:
                offset = -EINVAL;
                return offset;
        }
        
        file->f_pos = offset;
        MOD_DEBUG(KERN_DEBUG, "Seeking to position: %ld\n", \
                                (long)new_position);

}
//re-used from Max Holub's hivemod.c 
long cdev_ioctl (struct file *file, unsigned int ioctl_num,
                        unsigned long ioctl_param)
{
        struct hive_item *item = hive_tree_get_item(&hive_tree, file);
        if (NULL == item)
		return -EBADF;
        switch(ioctl_num) {
        case UPD_BUFFER:
                if(ioctl_param > item->length) {
                        char *buf = kzalloc(sizeof(*buf)*ioctl_param,
                                                GFP_KERNEL);
                        memcpy(buf, item->buffer,
                                sizeof(*buf)*item->length);
                        kfree(item->buffer);
                        item->buffer = buf;
                        item->length = ioctl_param;
                } else {
                        MOD_DEBUG(KERN_DEBUG, "No changes for buf");
                        return -1;
                }
                break;
        case ADD_MGC_PHR:
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
                break;


        }


}
// This structure is partially initialized here
// and the rest is initialized by the kernel after call
// to cdev_init()
// TODO: add ioctl to append magic phrase to buffer conents to
//       make these bees twerk
// TODO: add ioctl to select buffer size
static struct file_operations hive_fops = {
	.open =    &cdev_open,
	.release = &cdev_release,
	.read =    &cdev_read,
	.write =   &cdev_write,
        .llseek = &cdev_lseek,
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
	// paranoid cleanup (afterwards to ensure all fops ended)
	struct hive_item *item;
	for(struct rb_node *node = rb_first(&hive_tree); node; node = rb_next(node)) {
                item = rb_entry(node, struct hive_item, node);
		hive_tree_rm_item(item);
	}
}

static int __init cdevmod_init(void)
{
	int err = 0;
	
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

        if ((hive_class = class_create(THIS_MODULE, "hive_class")) == NULL) {
		unregister_chrdev_region(hive_dev, 1);
		return -1;
	}
	if (device_create(hive_class, NULL, hive_dev, NULL, "hive_dev") == NULL) {
		class_destroy(hive_class);
		unregister_chrdev_region(hive_dev, 1);
		return -1;
	}
	cdev_init(&hive_cdev, &hive_fops);
	// after call below the device becomes active
	// so all stuff should be initialized before
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
	MOD_DEBUG(KERN_DEBUG, "All honey reclaimed");
}
 
module_init(cdevmod_init);
module_exit(cdevmod_exit);
