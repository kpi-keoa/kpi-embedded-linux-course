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


MODULE_DESCRIPTION("Character dev");
MODULE_AUTHOR("Anton Kotsiubailo");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

#define MAGIC_NUMB              'N'

#define IOCTL_NEW_DATA_BUF_SZ           _IOW(MAGIC_NUMB, 2, unsigned long)
#define IOCTL_CH_MODE                   _IOW(MAGIC_NUMB, 3, bool)

#define LOW     true
#define HIGH    false


#define MOD_DEBUG(level, fmt, ...) \
	{pr_info(level "%s: " fmt "\n", THIS_MODULE->name,##__VA_ARGS__);}


struct alloc_status {
	unsigned long dev_created : 1;
	unsigned long cdev_added : 1;
};
// start with everything non-done
static struct alloc_status alloc_flags = { 0 };

/**
 * struct capital_flist_item - stores data for each descriptor
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
struct capital_ftree_item {
	struct rb_node tree_nd;
        struct file *file;
        char *databuff;
        int dat_b_sz;
        int dat_lenth;
        bool mode;

};



 struct rb_root my_capital_tree = RB_ROOT;

static char *devname = THIS_MODULE->name;
module_param(devname, charp, 0);
MODULE_PARM_DESC(devname, "Name as in VFS. Defaults to module name");
static int major = 0;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major number. Defaults to 0 (automatic allocation)");
static int buffsize_dat =  100;
//module_param(buffsize, int, 0);
//MODULE_PARM_DESC(buffsize, "Char buffer size. Defaults to 2 * sizeof magic_phrase");

dev_t capital_dev = 0;	// Stores our device handle
static struct cdev capital_cdev; // scull-initialized

/**
 * capital_flist_new() - creates list item having buffer
 * @buffer_size: numer of characters in buffer
 */
static inline struct capital_ftree_item *capital_ftree_new(unsigned long buffsized)
{
	// (!) here's where kernel memory (probably containing secrets) leaks
        // to userspace...
        // TODO: fix to make it zero'ed first (DONE)

        char *dat = kzalloc(sizeof(*dat) * buffsized, GFP_KERNEL);
	if (NULL == dat){
                return NULL;
        }

	struct capital_ftree_item *item = kzalloc(sizeof *item, GFP_KERNEL);
	if (NULL == item) {
                kfree(dat);
		return NULL;
	}

        item->databuff = dat;
        item->dat_b_sz = buffsize_dat;
        item->dat_lenth = 0; //test
        item->mode = HIGH;

	return item;
}

void capital(char *strin, char *strout,int flag) {
        int i;
	for(i = 0; i < strlen(strin); i++)
	{
		if (flag) {
			strout[i] = (strin[i] > 'Z')? strin[i]- 32 : strin[i];
		} else {
			strout[i] = (strin[i] <= 'Z')? strin[i] + 32 : strin[i];
		}
	}

}

/**
 * capital_flist_rm() - deletes item from list and frees memory
 * @item: list item
 */
static inline void capital_ftree_rm(struct capital_ftree_item *item)
{
	if (NULL == item)
		return;
	rb_erase(&item->tree_nd, &my_capital_tree);
	kfree(item->databuff);
	kfree(item);
}

/**
 * capital_flist_get - searches the list
 * @file: field of the list
 *
 * Return: item having the field or NULL if not found
 */
static struct capital_ftree_item *capital_ftree_get(struct rb_root *root, struct file *file)
{
        struct rb_node *node = root->rb_node;
        struct capital_ftree_item *tmp = NULL;
        while (NULL != node)
        {
                tmp = rb_entry(node, struct capital_ftree_item, tree_nd);

                if ((u64)(tmp->file) > (u64)file)
                        node = node->rb_left;
                else if ((u64)(tmp->file) < (u64)file)
                        node = node->rb_right;
                else {
                        return tmp;  /* Found it */
                }
        }
	// not found
	MOD_DEBUG(KERN_ERR, "Expected list entry not found %p", file);
	return NULL;
}

void capital_ftree_add(struct capital_ftree_item *new, struct rb_root *root)
{
        struct rb_node **next_p = &(root->rb_node);
        struct file *data = new->file;
        struct capital_ftree_item *tmp;
        struct rb_node *patern = NULL;
        // going down the tree
        while (NULL != (*next_p))
        {
                tmp = rb_entry(*next_p, struct capital_ftree_item, tree_nd);
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
                kfree(rb_entry(node_to_del, struct capital_ftree_item, tree_nd));
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
	struct capital_ftree_item *item = capital_ftree_new(buffsize_dat);
	if (NULL == item) {
		MOD_DEBUG(KERN_ERR, "Buffer allocate failed for %p", file);
		return -ENOMEM;
	}

	item->file = file;
	capital_ftree_add(item, &my_capital_tree);
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
	struct capital_ftree_item *item = capital_ftree_get(&my_capital_tree, file);
	if (NULL == item)
		return -EBADF;
	// remove item from list and free its memory
	capital_ftree_rm(item);
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
	struct capital_ftree_item *item = capital_ftree_get(&my_capital_tree, file);
	if (NULL == item)
		return -EBADF;
	// TODO: Add buffer read logic. Make sure seek operations work (DONE)
    	//       correctly
	//       Be careful not to access array above bounds
        int byts_rd = 0;
        //int nrbytes = 0;

        char kbuf[item->dat_lenth];
        char *kb_p = kbuf;

        capital(item->databuff, kbuf, item->mode);


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
	struct capital_ftree_item *item = capital_ftree_get(&my_capital_tree, file);
	if (NULL == item)
		return -EBADF;

	// TODO: Add buffer write logic. Make sure seek operations work (DONE)
    	//       correctly
	//       Be careful not to access array above bounds
        unsigned long buffsize = count;
        char *kbuf = kzalloc(buffsize, GFP_KERNEL);
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
                read++;
        }

        memset(item->databuff, 0, item->dat_b_sz);
        strcpy(item->databuff,kbuf);

        item->dat_lenth = count;

        kfree(kbuf);
	return read;
}

static long cdev_unl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct capital_ftree_item *item = capital_ftree_get(&my_capital_tree, file);
        if (NULL == item)
		return -EBADF;

        char tempd[arg];

        switch(cmd) {
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


static struct file_operations capital_fops = {
	.open =    &cdev_open,
	.release = &cdev_release,
	.read =    &cdev_read,
	.write =   &cdev_write,
        .unlocked_ioctl =   &cdev_unl_ioctl,
	.owner =   THIS_MODULE,
};


static void module_cleanup(void)
{

	if (alloc_flags.cdev_added) {
		cdev_del(&capital_cdev);
	}
	if (alloc_flags.dev_created) {
		unregister_chrdev_region(capital_dev, 1);
	}

	del_tree(&my_capital_tree);
}

static int __init cdevmod_init(void)
{
	int err = 0;

	if (0 == major) {
		// use dynamic allocation (automatic)
		err = alloc_chrdev_region(&capital_dev, 0, 1, devname);
	} else {
		// stick with what user provided
		capital_dev = MKDEV(major, 0);
		err = register_chrdev_region(capital_dev, 1, devname);
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
	          MAJOR(capital_dev), MINOR(capital_dev));


	cdev_init(&capital_cdev, &capital_fops);

	if ((err = cdev_add(&capital_cdev, capital_dev, 1))) {
		MOD_DEBUG(KERN_ERR, "Add cdev failed with %d", err);
		goto err_handler;
	}
	alloc_flags.cdev_added = 1;


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
