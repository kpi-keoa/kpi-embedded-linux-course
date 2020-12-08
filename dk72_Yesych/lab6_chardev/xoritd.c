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
#include <linux/string.h>
#include <linux/types.h>


MODULE_DESCRIPTION("Character device xor encoder");
MODULE_AUTHOR("Dm1Tr0");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

#define MAGIC_NUMB              'A'

#define IOCTL_NEW_KEY_BUF_SZ            _IOW(MAGIC_NUMB, 1, unsigned long)
#define IOCTL_NEW_DATA_BUF_SZ           _IOW(MAGIC_NUMB, 2, unsigned long)
#define IOCTL_CH_MODE                   _IOW(MAGIC_NUMB, 3, bool)


#define KEY     false
#define DATA    true


 // correct all xorits to xorit, change read black tree to mbeded file structure
 // implement size cottection of buffers and mode change, and ofcoures check how this shit works
 // it would be smart to do some logging before testing the module it could be easely remooved using sed


/**
 * MOD_DEBUG(level, fmt, ...) - module debug printer
 * @level: printk debug level (e.g. KERN_INFO)
 * @fmt: printk format
 *
 * Prints message using pr_info(), adding module name
 */
#define MOD_DEBUG(level, fmt, ...) \
	{pr_info(level "%s: " fmt "\n", THIS_MODULE->name,##__VA_ARGS__);}
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
 * struct xorit_flist_item - stores data for each descriptor
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
struct xorit_ftree_item {
	struct rb_node tree_nd;
        struct file *file;
        char *keybuff;
        char *databuff;
        int dat_b_sz;
        int key_b_sz;
        int key_lenth;
        int dat_lenth;
        bool mode;

};



 struct rb_root my_xorit_tree = RB_ROOT;

static char *devname = THIS_MODULE->name;
module_param(devname, charp, 0);
MODULE_PARM_DESC(devname, "Name as in VFS. Defaults to module name");
static int major = 0;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major number. Defaults to 0 (automatic allocation)");
static int buffsize_key = 100;
static int buffsize_dat =  100;
//module_param(buffsize, int, 0);
//MODULE_PARM_DESC(buffsize, "Char buffer size. Defaults to 2 * sizeof magic_phrase");

dev_t xorit_dev = 0;	// Stores our device handle
static struct cdev xorit_cdev; // scull-initialized

/**
 * xorit_flist_new() - creates list item having buffer
 * @buffer_size: numer of characters in buffer
 */
static inline struct xorit_ftree_item *xorit_ftree_new(unsigned long buffsized, unsigned long buffsizek)
{
	// (!) here's where kernel memory (probably containing secrets) leaks
        // to userspace...
        // TODO: fix to make it zero'ed first (DONE)
        char *key = kzalloc(sizeof(*key) * buffsizek, GFP_KERNEL);
        if (NULL == key) {

                return NULL;
        }

        char *dat = kzalloc(sizeof(*dat) * buffsized, GFP_KERNEL);
	if (NULL == dat){
                kfree(key);
                return NULL;
        }
        strcpy(dat, "one two thre"); //test

	struct xorit_ftree_item *item = kzalloc(sizeof *item, GFP_KERNEL);
	if (NULL == item) {
                kfree(key);
		return NULL;
	}

        item->databuff = dat;
        item->keybuff = key;
        item->dat_b_sz = buffsize_dat;
        item->key_b_sz = buffsize_key;
        item->key_lenth = 0;
        item->dat_lenth = strlen(dat)+1; //test
        item->mode = DATA;

	return item;
}

void xorit(char *data, char *key, char *output, int dataLen, int keyLen) {
        keyLen = keyLen ? keyLen : 1; // in case of lenth = 0

        int i;
	for (i = 0; i < dataLen; i++) {
		output[i] = data[i] ^ key[i % keyLen];
	}
        output[i] = '\0';

}


/**
 * xorit_flist_rm() - deletes item from list and frees memory
 * @item: list item
 */
static inline void xorit_ftree_rm(struct xorit_ftree_item *item)
{
	if (NULL == item)
		return;
	rb_erase(&item->tree_nd, &my_xorit_tree);
	kfree(item->databuff);
        kfree(item->keybuff);
	kfree(item);
}

/**
 * xorit_flist_get - searches the list
 * @file: field of the list
 *
 * Return: item having the field or NULL if not found
 */
static struct xorit_ftree_item *xorit_ftree_get(struct rb_root *root, struct file *file)
{
        struct rb_node *node = root->rb_node;
        struct xorit_ftree_item *tmp = NULL;
        while (NULL != node)
        {
                tmp = rb_entry(node, struct xorit_ftree_item, tree_nd);

                if ((u64)(tmp->file) > (u64)file)
                        node = node->rb_left;
                else if ((u64)(tmp->file) < (u64)file)
                        node = node->rb_right;
                else {
                        pr_info(KERN_ERR "search: found %p val\n", file);
                        return tmp;  /* Found it */
                }
        }
	// not found
	MOD_DEBUG(KERN_ERR, "Expected list entry not found %p", file);
	return NULL;
}

void xorit_ftree_add(struct xorit_ftree_item *new, struct rb_root *root)
{
        struct rb_node **next_p = &(root->rb_node);
        struct file *data = new->file;
        struct xorit_ftree_item *tmp;
        struct rb_node *patern = NULL;
        // going down the tree
        while (NULL != (*next_p))
        {
                tmp = rb_entry(*next_p, struct xorit_ftree_item, tree_nd);
                patern = *next_p;

                if ((u64)(tmp->file) > (u64)data)
                        next_p = &((*next_p)->rb_left);
                else if ((u64)(tmp->file) < (u64)data)
                        next_p = &((*next_p)->rb_right);
                else {
                        pr_info(KERN_ERR "insertion error\n");
                        return;
                }
        }
        //struct rb_node *patern = &(*tmp).tree_nd;
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
                kfree(rb_entry(node_to_del, struct xorit_ftree_item, tree_nd));
                node_to_del = node_tmp;
                cnt++;
        }
        pr_info("del_tree>>\ncnt = %d\n", cnt);
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
	struct xorit_ftree_item *item = xorit_ftree_new(buffsize_dat, buffsize_key);
	if (NULL == item) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		return -ENOMEM;
	}

	item->file = file;
	xorit_ftree_add(item, &my_xorit_tree);
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
	struct xorit_ftree_item *item = xorit_ftree_get(&my_xorit_tree, file);
	if (NULL == item)
		return -EBADF;
	// remove item from list and free its memory
	xorit_ftree_rm(item);
	MOD_DEBUG(KERN_DEBUG, "File entry %p unlinked", file);
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
	struct xorit_ftree_item *item = xorit_ftree_get(&my_xorit_tree, file);
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer read logic. Make sure seek operations work (DONE)
    	//       correctly
	//       Be careful not to access array above bounds
        int byts_rd = 0;
        //int nrbytes = 0;

        char kbuf[item->dat_lenth];
        count = item->dat_lenth;

        char *kb_p = kbuf;

        pr_info("cdev_write>> checking the buffers of the structure hrere\n"
                "dat_b_sz = %d\nkey_b_sz = %d\ndat_lenth = %d\nkey_lenth = %d"
                "\nmode = %d(0 = KEY)\n", item->dat_b_sz, item->key_b_sz,
                 item->dat_lenth, item->key_lenth, item->mode);

        xorit(item->databuff, item->keybuff, kbuf, item->dat_lenth,
                 item->key_lenth);


        while (count && *kb_p) {
                if (0 != put_user(*(kb_p++), buf++)) {
                        MOD_DEBUG(KERN_DEBUG, "error while writing to usr\n");
                        return -EFAULT;
                }
                byts_rd++;
                count--;
        }


        memset(kbuf, 0, sizeof(kbuf));

	return byts_rd;
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
	struct xorit_ftree_item *item = xorit_ftree_get(&my_xorit_tree, file);
	if (NULL == item)
		return -EBADF;

	// TODO: Add buffer write logic. Make sure seek operations work (DONE)
    	//       correctly
	//       Be careful not to access array above bounds
        unsigned long buffsize = count;
        char *kbuf = kzalloc(buffsize, GFP_KERNEL);
        pr_info("cdev_write>> checking the buffers of the structure hrere\n"
                "dat_b_sz = %d\nkey_b_sz = %d\ndat_lenth = %d\nkey_lenth = %d"
                "\nmode = %d(0 = KEY)\n", item->dat_b_sz, item->key_b_sz,
                 item->dat_lenth, item->key_lenth, item->mode);
        /* Thus by substruction of the pointers we get ammont of bytes that was
         * writen to the kbuffer.
         */
        unsigned long read = 0;
        int i = 0;
        while (read < buffsize){
                if (0 != get_user(kbuf[read], buf++) ) {
                        MOD_DEBUG(KERN_DEBUG, "error while reading from usr\n");
                        return -EFAULT;
                }
                pr_info("%c ", kbuf[read]);
                pr_info("%d ", read);
                read++;
        }


        if (item->mode == KEY) {
                memset(item->keybuff, 0, item->key_b_sz);
                strcpy(item->keybuff, kbuf);
                pr_info("cdev_write>> %s key str that goes into the struct",
                         item->keybuff);

                item->key_lenth = count;
        } else {
                memset(item->databuff, 0, item->dat_b_sz);
                strcpy(item->databuff,kbuf);
                pr_info("cdev_write>> %s data str that goes into the struct",
                         item->databuff);

                item->dat_lenth = count;
        }

        kfree(kbuf);
        pr_info("cdev_write>>\n read = %ld", read);
	return read;
}

static long cdev_unl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct xorit_ftree_item *item = xorit_ftree_get(&my_xorit_tree, file);
        if (NULL == item)
		return -EBADF;

        char tempk[arg];
        char tempd[arg];

        switch(cmd) {
        case IOCTL_NEW_KEY_BUF_SZ:

                strcpy(tempk, item->keybuff);
                if(NULL == krealloc(item->keybuff, arg, GFP_KERNEL)) {
			pr_err("cdev_unl_ioctl>>Counldn't reallocate memory\n");
			return -ENOMEM;
		}
                strcpy(item->databuff, tempk);
                item->key_b_sz = arg;

                break;

        case IOCTL_NEW_DATA_BUF_SZ:

                strcpy(tempd, item->databuff);
                if(NULL == krealloc(item->databuff, arg, GFP_KERNEL)) {
                        pr_err("cdev_unl_ioctl>>Counldn't reallocate memory\n");
                        return -ENOMEM;
                }
                strcpy(item->databuff, tempd);
                item->dat_b_sz = arg;

                break;

        case IOCTL_CH_MODE:


                if((arg == true)||(arg == false))
                        item->mode = arg;
                else
                        pr_err("cdev_unl_ioctl>>wrong argument \n");

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
static struct file_operations xorit_fops = {
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
		cdev_del(&xorit_cdev);
	}
	if (alloc_flags.dev_created) {
		unregister_chrdev_region(xorit_dev, 1);
	}
	// paranoid cleanup (afterwards to ensure all fops ended)
	del_tree(&my_xorit_tree);
}

static int __init cdevmod_init(void)
{
	int err = 0;

	if (0 == major) {
		// use dynamic allocation (automatic)
		err = alloc_chrdev_region(&xorit_dev, 0, 1, devname);
	} else {
		// stick with what user provided
		xorit_dev = MKDEV(major, 0);
		err = register_chrdev_region(xorit_dev, 1, devname);
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
	          MAJOR(xorit_dev), MINOR(xorit_dev));


	cdev_init(&xorit_cdev, &xorit_fops);
	// after call below the device becomes active
	// so all stuff should be initialized before
	if ((err = cdev_add(&xorit_cdev, xorit_dev, 1))) {
		MOD_DEBUG(KERN_ERR, "Add cdev failed with %d", err);
		goto err_handler;
	}
	alloc_flags.cdev_added = 1;
	// TODO: add stuff here to make module register itself in /dev yet to implement

	return 0;

err_handler:
	module_cleanup();
	return err;
}

static void __exit cdevmod_exit(void)
{
	module_cleanup();

}

module_init(cdevmod_init);
module_exit(cdevmod_exit);
