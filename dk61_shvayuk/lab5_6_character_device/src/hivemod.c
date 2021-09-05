#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>       // to allow registration of device at /dev
#include <linux/uaccess.h>      // for get_user and put_user functions
#include <linux/types.h>        // for using the size_t type
#include <linux/rbtree.h>

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("max shvayuk");
MODULE_VERSION("0.228");
MODULE_LICENSE("Dual MIT/GPL");

#define IOCTL_TWERK               ((unsigned int)0)
#define IOCTL_SET_NEW_BUFFER_SIZE ((unsigned int)1)

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
	unsigned long dev_created       : 1;
	unsigned long cdev_added        : 1;
        unsigned long udev_class_added  : 1;
        unsigned long udev_device_added : 1;
};
// start with everything non-done
static struct alloc_status alloc_flags = { 0 };

/**
 * struct hive_ftree_item - stores data for each descriptor
 * @list: fields to link the list
 * @file: created on open(), removed on close()
 *        changes during file operations, but ptr stays the same
 * @buffer: memory we allocate for each file
 * @length: buffer size
 * @rdoffset: reader offset
 * @wroffset: writer offset
 *
 * TODO: change this to proper associative array or tree
 */
struct hive_ftree_item {
// 	struct list_head list;
        struct rb_node tree_node;
	struct file *file;
	char *buffer;
	long length;
	long rdoffset;
	long wroffset;
};

struct rb_root hive_tree_root = RB_ROOT;

static const char magic_phrase[] = "Wow, we made these bees TWERK !";
static const char udev_class_name[] = "hive_test_dev_class";
static const char udev_device_name[] = "hive_test_device";

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
static struct class *hive_udev_class = NULL;
static struct device *hive_udev_device = NULL;

/**
 * hive_ftree_new() - creates rbtree item having buffer
 * @buffer_size: numer of characters in buffer
 */
static inline struct hive_ftree_item *hive_ftree_new(unsigned long buffer_size)
{
        // TODO: fix to make it zero'ed first
	char *buf = kzalloc(sizeof(*buf) * buffsize, GFP_KERNEL);
	if (NULL == buf)
		return NULL;

	struct hive_ftree_item *item = kmalloc(sizeof *item, GFP_KERNEL);
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
 * hive_ftree_rm() - deletes item from rbtree and frees memory
 * @item: tree item
 */
static inline void hive_ftree_rm(struct hive_ftree_item *item)
{
	if (NULL == item)
		return;
	rb_erase(&item->tree_node, &hive_tree_root);
	kfree(item->buffer);
	kfree(item);
}

/**
 * delete_rbtree() - deletes thw whole rbtree and frees memory
 * @root: ptr to the tree's root
 */
int delete_rbtree(struct rb_root *root)
{
        int cnt = 0;
        struct rb_node *node_it = rb_first(root);
        struct rb_node *node_tmp = NULL;
        while (node_it) {
                node_tmp = rb_next(node_it);
                rb_erase(node_it, root);
                kfree(rb_entry(node_it, struct hive_ftree_item, tree_node));
                node_it = node_tmp;
                cnt++;
        }
        return cnt;
}

/**
 * hive_ftree_get - searches the tree
 * @file: field of the tree
 *
 * Return: item having the field or NULL if not found
 */
static struct hive_ftree_item *hive_ftree_get(struct rb_root *root, struct file *file)
{
	struct hive_ftree_item *tmp = NULL;
        struct rb_node *node = root->rb_node;
        
        while (node)
        {
                tmp = rb_entry(node, struct hive_ftree_item, tree_node);

                if ((uint64_t)(tmp->file) > (uint64_t)file)
                        node = node->rb_left;
                else if ((uint64_t)(tmp->file) < (uint64_t)file)
                        node = node->rb_right;
                else {
                        printk(KERN_ERR "search: found %p val\n", file);
                        return tmp;  /* Found it */
                }
        }
        
	// not found
	MOD_DEBUG(KERN_ERR, "Expected rbtree entry not found: %p", file);
	return NULL;
}

void hive_ftree_add(struct rb_root *root, struct hive_ftree_item *new)
{        
        struct rb_node **link = &(root->rb_node);
        struct rb_node *parent = NULL;
        struct file *data = new->file;
        
        /* Go to the bottom of the tree */
        while (*link)
        {
                parent = *link;
                struct hive_ftree_item *tmp = rb_entry(parent, struct hive_ftree_item, tree_node);

                if ((uint64_t)(tmp->file) > (uint64_t)data)
                        link = &((*link)->rb_left);
                else if ((uint64_t)(tmp->file) < (uint64_t)data)
                        link = &((*link)->rb_right);
                else {
                        printk(KERN_ERR "insert error\n");
                        return;
                }
        }

        /* Put the new node there */
        rb_link_node(&(new->tree_node), parent, link);
        rb_insert_color(&(new->tree_node), root);
}


// For more, see LKD 3rd ed. chapter 13
/**
 * cdev_open() - callback for open() file operation
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 *
 * Allocates memory, creates fd entry and adds it to rbtree
 */
static int cdev_open(struct inode *inode, struct file *file)
{
	struct hive_ftree_item *item = hive_ftree_new(buffsize);
	if (NULL == item) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		return -ENOMEM;
	}
	// fill the rest
	item->file = file;
        hive_ftree_add(&hive_tree_root ,item);
        
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
	struct hive_ftree_item *item = hive_ftree_get(&hive_tree_root, file);
	if (NULL == item)
		return -EBADF;
	// remove item from rbtree and free its memory
        hive_ftree_rm(item);
        
	MOD_DEBUG(KERN_DEBUG, "File entry %p unlinked", file);
	return 0;
}

/**
 * cdev_read() - called on file read() operation
 * @file:   VFS file opened by a process
 * @buf:    The buffer to fill with data
 * @length: The length of the buffer 
 * @offset: Our offset in the file 
 */
static ssize_t cdev_read(struct file *file, char __user *buf, 
			 size_t length, loff_t *offset)
{  
	struct hive_ftree_item *item = hive_ftree_get(&hive_tree_root, file);
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer read logic.
        char *ptr_kern_buff = item->buffer;
        int bytes_read = 0;
        while (length && *ptr_kern_buff) {
                if (0 != put_user(*ptr_kern_buff, buf++) ) {
                        return -1;
                }
                length--;
                bytes_read++;
                ptr_kern_buff++;
        }
        
	return bytes_read;
}

/**
 * cdev_write() - callback for file write() operation
 * @file: VFS file opened by a process
 * @buf:        
 * @length:

 * @offset:
 */
static ssize_t cdev_write(struct file *file, const char __user *buf,
			  size_t length, loff_t *offset)
{
	struct hive_ftree_item *item = hive_ftree_get(&hive_tree_root, file);
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer write logic.
        if (length > buffsize)
                length = buffsize;
        int cnt = 0;
        while (cnt < length) {
                if (0 != get_user((item->buffer)[cnt], buf) ) {
                        MOD_DEBUG(KERN_DEBUG, "write error\n");
                        return -1;
                }
                buf++;
                cnt++;
        }
        
	return cnt;
}

/**
 * cdev_ioctl() - callback for file ioctl() operation
 * @file:   VFS file opened by a process
 * @cmd: 
 * @arg:
 */
static long cdev_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct hive_ftree_item *item = hive_ftree_get(&hive_tree_root, file);
	if (NULL == item)
		return -EBADF;
        
        switch (cmd) {
        case IOCTL_TWERK:
                /* 
                 * I use "1;" line to avoid "error: a label can only be part
                 * of a statement and a declaration is not a statement" error 
                 */
                1;
                size_t magic_phrase_len = strlen(magic_phrase);
                if (magic_phrase_len > item->length) {
                        MOD_DEBUG(KERN_DEBUG, "ioctl: the magic phrase is too"
                                  " long, increase the buffer size first\n");
                        return -1;
                }
                for (size_t i = 0; i < magic_phrase_len; i++)
                        (item->buffer)[i] = magic_phrase[i];
                break;
        case IOCTL_SET_NEW_BUFFER_SIZE:
                1;
                /* arg is new buffer size */
                char *new_buffer = kmalloc(sizeof(*new_buffer) * arg, GFP_KERNEL);
                if ('\0' != (item->buffer)[0]) {
                        /* to avoid new buffer overflow */
                        if (arg < item->length)
                                (item->buffer)[arg-1] = '\0';
                        char *tmp = item->buffer;
                        item->buffer = strcpy(new_buffer, item->buffer);
                        item->length = arg;
                        kfree(tmp);
                        MOD_DEBUG(KERN_DEBUG, "ioctl: new buffer size = %li \n", item->length);     
                }
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
	.open             = &cdev_open,
	.release          = &cdev_release,
	.read             = &cdev_read,
	.write            = &cdev_write,
        .unlocked_ioctl   = &cdev_unlocked_ioctl,
	// required to prevent module unloading while fops are in use
	.owner =   THIS_MODULE,
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
	/* Don't change the order of next two if-statements */
        if (alloc_flags.udev_device_added) {
                device_destroy(hive_udev_class, hive_dev);
        }
	if (alloc_flags.udev_class_added) {
                class_destroy(hive_udev_class);
        }
	// paranoid cleanup (afterwards to ensure all fops ended)
        delete_rbtree(&hive_tree_root);
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

	cdev_init(&hive_cdev, &hive_fops);
	// after call below the device becomes activnevere
	// so all stuff should be initialized before
	if ((err = cdev_add(&hive_cdev, hive_dev, 1))) {
		MOD_DEBUG(KERN_ERR, "Add cdev failed with %d", err);
		goto err_handler;
	}
	alloc_flags.cdev_added = 1;
	MOD_DEBUG(KERN_DEBUG, "This hive has %lu bees", 2 + jiffies % 8);
	
        // TODO: add stuff here to make module register itself in /dev
        hive_udev_class = class_create(THIS_MODULE, udev_class_name);
        if (IS_ERR(hive_udev_class)) {
                err = PTR_ERR(hive_udev_class);
                goto err_handler;
        }
        alloc_flags.udev_class_added = 1;
        hive_udev_device = device_create(hive_udev_class,
                                         NULL, 
                                         hive_dev, 
                                         NULL,
                                         udev_device_name,
                                         MINOR(hive_dev));
        if (IS_ERR(hive_udev_device)) {
                err = PTR_ERR(hive_udev_device);
                goto err_handler;
        }
        alloc_flags.udev_device_added = 1;
        
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
