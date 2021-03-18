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

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("thodnev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

#define MAGIC_NUMB              'A'
#define IOCTL_PHRASE             _IO(MAGIC_NUMB, 1)
#define IOCTL_NEW_BUFF_SZ        _IOW(MAGIC_NUMB, 2, int*)


u64 beeval = 12;
struct dentry *ddir = NULL, *dfile = NULL;

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
 * TODO: change this to proper associative array or tree (DONE)
 */
struct hive_ftree_item {
	struct rb_node tree_nd;
	struct file *file;
	char *buffer;
	long length;
	long rdoffset;
	long wroffset;
};

 struct rb_root my_hive_tree = RB_ROOT;

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

/**
 * hive_flist_new() - creates list item having buffer
 * @buffer_size: numer of characters in buffer
 */
static inline struct hive_ftree_item *hive_ftree_new(unsigned long buffer_size)
{
	// (!) here's where kernel memory (probably containing secrets) leaks
        // to userspace...
        // TODO: fix to make it zero'ed first (DONE)
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
 * hive_flist_rm() - deletes item from list and frees memory
 * @item: list item
 */
static inline void hive_ftree_rm(struct hive_ftree_item *item)
{
	if (NULL == item)
		return;
	rb_erase(&item->tree_nd, &my_hive_tree);
	kfree(item->buffer);
	kfree(item);
}

/**
 * hive_flist_get - searches the list
 * @file: field of the list
 *
 * Return: item having the field or NULL if not found
 */
static struct hive_ftree_item *hive_ftree_get(struct rb_root *root, struct file *file)
{
        struct rb_node *node = root->rb_node;
        struct hive_ftree_item *tmp = NULL;

        while (NULL != node)
        {
                tmp = rb_entry(node, struct hive_ftree_item, tree_nd);

                if ((u64)(tmp->file) > (u64)file)
                        node = node->rb_left;
                else if ((u64)(tmp->file) < (u64)file)
                        node = node->rb_right;
                else {
                        printk(KERN_ERR "search: found %p val\n", file);
                        return tmp;  /* Found it */
                }
        }
	// not found
	MOD_DEBUG(KERN_ERR, "Expected list entry not found %p", file);
	return NULL;
}

void hive_ftree_add(struct hive_ftree_item *new, struct rb_root *root)
{
        struct rb_node **next_p = &(root->rb_node);
        struct file *data = new->file;
        struct hive_ftree_item *tmp;

        // going down the tree
        while (NULL != (*next_p))
        {
                tmp = rb_entry(*next_p, struct hive_ftree_item, tree_nd);

                if ((u64)(tmp->file) > (u64)data)
                        next_p = &((*next_p)->rb_left);
                else if ((u64)(tmp->file) < (u64)data)
                        next_p = &((*next_p)->rb_right);
                else {
                        printk(KERN_ERR "insertion error\n");
                        return;
                }
        }

        struct rb_node *patern = &(*tmp).tree_nd;
        rb_link_node(&(new->tree_nd),patern ,next_p);
        rb_insert_color(&(new->tree_nd), root);
}

int del_tree(struct rb_root *root)
{
        int cnt = 0;
        struct rb_node *node_to_del = rb_first(root);
        struct rb_node *node_tmp = NULL;
        while ( NULL != node_to_del) {
                node_tmp = rb_next(node_to_del);
                rb_erase(node_to_del, root);
                kfree(rb_entry(node_to_del, struct hive_ftree_item, tree_nd));
                node_to_del = node_tmp;
                cnt++;
        }
        return cnt;
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
	struct hive_ftree_item *item = hive_ftree_new(buffsize);
	if (NULL == item) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		return -ENOMEM;
	}

	// fill the rest
	item->file = file;
	hive_ftree_add(item, &my_hive_tree);

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
	struct hive_ftree_item *item = hive_ftree_get(&my_hive_tree, file);
	if (NULL == item)
		return -EBADF;
	// remove item from list and free its memory
	hive_ftree_rm(item);
	MOD_DEBUG(KERN_DEBUG, "File entry %p unlinked", file);
	return ;
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
	struct hive_ftree_item *item = hive_ftree_get(&my_hive_tree, file);
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer read logic. Make sure seek operations work (DONE)
    	//       correctly
	//       Be careful not to access array above bounds
        int byts_rd = 0;
        char *kbuf_p = item->buffer;
        while (count && *kbuf_p) {
                if (0 != put_user(*kbuf_p, buf++)) {
                        MOD_DEBUG(KERN_DEBUG, "error while writing to usr\n");
                        return -EFAULT;
                }
                byts_rd++;
                count--;
                kbuf_p++;
        }

	return 0;
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
	struct hive_ftree_item *item = hive_ftree_get(&my_hive_tree, file);
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer write logic. Make sure seek operations work (DONE)
    	//       correctly
	//       Be careful not to access array above bounds
        count = buffsize < count ? buffsize : count;

        char *kbuf_p = item->buffer;
        /* Thus by substruction of the pointers we get ammont of bytes that was
         * writen to the kbuffer.
         */
        while (((int)(kbuf_p - item->buffer)) < count) {
                if (0 != get_user(kbuf_p, buf) ) {
                        MOD_DEBUG(KERN_DEBUG, "error while reading from usr\n");
                        return -EFAULT;
                        kbuf_p++;
                        buf++;
                }
        }

	return kbuf_p - item->buffer;
}

static long cdev_unl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

        //taking to a count \0 at the end of a line
        size_t magic_phr_l = sizeof(magic_phrase);
        char temp[arg];

        struct hive_ftree_item *item = hive_ftree_get(&my_hive_tree, file);
        if (NULL == item)
		return -EBADF;

        size_t buf_size = item->length;

        switch(cmd) {
        case IOCTL_NEW_BUFF_SZ:

                strcpy(temp, item->buffer);
                if(NULL == krealloc(item->buffer, arg, GFP_KERNEL)) {
			pr_err("Counldn't reallocate memory\n");
			return -ENOMEM;
		}
                strcpy(item->buffer, temp);

                break;
        case IOCTL_PHRASE:

                if (magic_phr_l > buf_size) {
                        MOD_DEBUG(KERN_DEBUG, "ioctl: the magic phrase is too"
                        " long, increase the buf\n");
                        return -EFAULT;

                }
                int i;
                for (i = 0; i < magic_phr_l; i++) {
                        //taking to a count \0 at the end of a line
                        (item->buffer)[i] = magic_phrase[i];
                }
                break;
        default:
                return -ENOTTY;
        }

        return 0;
}


// This structure is partially initialized here
// and the rest is initialized by the kernel after call
// to cdev_init()
// TODO: add ioctl to append magic phrase to buffer conents to (DONE)
//       make these bees twerk
// TODO: add ioctl to select buffer size (DONE)
static struct file_operations hive_fops = {
	.open =    &cdev_open,
	.release = &cdev_release,
	.read =    &cdev_read,
	.write =   &cdev_write,
        .unlocked_ioctl =   &cdev_unl_ioctl,
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
	// paranoid cleanup (afterwards to ensure all fops ended)
	del_tree(&my_hive_tree);
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
	alloc_flags.cdev_added = 1;
	MOD_DEBUG(KERN_DEBUG, "This hive has %lu bees", 2 + jiffies % 8);
	// TODO: add stuff here to make module register itself in /dev yet to implement

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
