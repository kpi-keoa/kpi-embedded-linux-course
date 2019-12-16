#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/rbtree.h>

MODULE_DESCRIPTION("Character device demo");
MODULE_AUTHOR("thodnev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

#define IOCTL_BEE_TWERK 0
#define IOCTL_CHG_BUF_SIZE 1

/**
 * MOD_DEBUG(level, fmt, ...) - module debug printer
 * @level: printk debug level (e.g. KERN_INFO)
 * @fmt: printk format
int register_chrdev (unsigned int   major,
                     const char *   name,
                     const struct   fops);
                     file_operations *
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
 */
struct hive_ftree_item {
	struct rb_node node;
	struct file *file;
	char *buffer;
	long length;
	long rdoffset;
	long wroffset;
	struct debugfs_blob_wrapper blob_wrapper;
	struct dentry *blob_entry;
};


struct rb_root tree = RB_ROOT;
struct dentry *dirret = NULL;
int current_file_name_num = 0;

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
struct class *hive_class = NULL;


//static inline struct hive_ftree_item *hive_ftree_new(unsigned long buffer_size);
//static inline void hive_ftree_rm(struct hive_ftree_item *item);
//struct hive_ftree_item *hive_ftree_get(struct rb_root *root, struct file *file);

//int insert_node(struct rb_root *root, struct hive_ftree_item *insert_data);
//void delete_tree(struct rb_root *tree);

/**
 * insert_node() - insert node to tree
 * @root: root of tree 
 * @insert_data: node for insertion
 */
int insert_node(struct rb_root *root, struct hive_ftree_item *insert_data)
{
	struct rb_node **current_link = &(root->rb_node);
	struct rb_node *parent = NULL;
	
	uint64_t data = (uint64_t)(insert_data->file);
      
	while (*current_link) {
		struct hive_ftree_item *this = rb_entry(*current_link, struct hive_ftree_item, node);
      	uint64_t current_data = (uint64_t)(this->file);

      	parent = *current_link;
     	if (data < current_data) {
        	current_link = &((*current_link)->rb_left);
        }
        else if (data > current_data) {
        	current_link = &((*current_link)->rb_right);
        }
        else {
        	return -1;
        }
	}

	rb_link_node(&insert_data->node, parent, current_link);
	rb_insert_color(&insert_data->node, root);
	return 0;
}

/**
 * delete_tree() - delete all nodes from tree and free memory
 * @tree: root of tree for delete
 */
void delete_tree(struct rb_root *tree)
{
	struct rb_node *node = rb_first(tree);
	struct rb_node *tmp_node = NULL;
	struct hive_ftree_item *this = NULL;
	
	while(node) {
		tmp_node = node;
		node = rb_next(node);
		this = rb_entry(tmp_node, struct hive_ftree_item, node);
		
		printk("DELETE=%llu\n", (uint64_t)this->file);
		
		rb_erase(tmp_node, tree);
    	kfree(this);	
	}
}

/**
 * hive_ftree_new() - creates tree item having buffer
 * @buffer_size: numer of characters in buffer
 */
static inline struct hive_ftree_item *hive_ftree_new(unsigned long buffer_size)
{
	char *buf = kzalloc(sizeof(*buf) * buffer_size, GFP_KERNEL);
	if (NULL == buf)
		return NULL;

	struct hive_ftree_item *item = kzalloc(sizeof *item, GFP_KERNEL);
	if (NULL == item) {
		kfree(buf);	// avoid mem leaks
		return NULL;
	}

	item->buffer = buf;
	item->length = buffer_size;
	item->rdoffset = 0;
	item->wroffset = 0;
	item->blob_wrapper.data = (void *)buf;
	item->blob_wrapper.size = buffer_size;
	return item;
}

/**
 * hive_ftree_rm() - deletes item from tree and frees memory
 * @item: tree item
 * @tree: root of tree
 */
static inline void hive_ftree_rm(struct rb_root *tree, struct hive_ftree_item *item)
{
	if (NULL == item)
		return;

	rb_erase(&(item->node), tree);
	kfree(item->buffer);
	kfree(item);
}

/**
 * hive_flist_get - searches the list
 * @file: field of the list
 *
 * Return: item having the field or NULL if not found
 */
struct hive_ftree_item *hive_ftree_get(struct rb_root *root, struct file *file)
{
	struct rb_node *current_node = root->rb_node;
	
	while (current_node) {
		struct hive_ftree_item *this = rb_entry(current_node, struct hive_ftree_item, node);
      	uint64_t current_data = (uint64_t)(this->file);

		if(current_data == (uint64_t)file) {
			return rb_entry(current_node, struct hive_ftree_item, node);
		}
     	else if ((uint64_t)file < current_data) {
        	current_node = current_node->rb_left;
        }
        else if ((uint64_t)file > current_data) {
        	current_node = current_node->rb_right;
        }
        else {
        	return NULL;
        }
	}


	return NULL;
}


void int_to_string(int src, char *str, unsigned int size)
{
	int digit = 0;
	
	if (size <= 0 || str == NULL)
		return;
		
	int i = 0;
	while((i < (size - 1))) {
		digit = src % 10;
		str[i] = '0' + digit;
		src /= 10;	
		i++;
	}
	
	int begin_pointer = 0;
	int end_pointer = size - 2;
	
	while(begin_pointer <= end_pointer) {
		char tmp = str[end_pointer];
		str[end_pointer] = str[begin_pointer];
		str[begin_pointer] = tmp;
		begin_pointer++;
		end_pointer--;
	}
	
	str[size - 1] = '\0';
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
	insert_node(&tree, item);
	
	char str[11];
	int_to_string(current_file_name_num, str, 11);
	current_file_name_num++;

	item->blob_entry = debugfs_create_blob(str, 0777, dirret, &(item->blob_wrapper));
	
	MOD_DEBUG(KERN_DEBUG, "New file entry %p created, debugfs name - %s", file, str);

	return 0;
}

/**
 * cdev_release() - file close() callback
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 */
static int cdev_release(struct inode *inode, struct file *file)
{
	struct hive_ftree_item *item = hive_ftree_get(&tree, file);
	if (NULL == item)
		return -EBADF;
	// remove item from list and free its memory
	debugfs_remove(item->blob_entry);
	hive_ftree_rm(&tree, item);
	MOD_DEBUG(KERN_DEBUG, "File entry %p unlinked", file);
	return 0;
}

/**
 * cdev_read() - called on file read() operation
 * @file: VFS file opened by a process
 * @buf: buffer from user space
 * @count: number of bytes received from user space
 * @loff: file offset, currently not use
 */
static ssize_t cdev_read(struct file *file, char __user *buf, 
			 size_t count, loff_t *loff)
{
	struct hive_ftree_item *item = hive_ftree_get(&tree, file);
	if (NULL == item)
		return -EBADF;
	
	if(item->length < count)
		count = item->length;
	
	copy_to_user(buf, item->buffer, count);
	return count;
}

/**
 * cdev_write() - callback for file write() operation
 * @file: VFS file opened by a process
 * @buf: buffer from user space
 * @count: number of bytes received from user space
 * @loff: file offset, currently not use
 */
static ssize_t cdev_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *loff)
{
	struct hive_ftree_item *item = hive_ftree_get(&tree, file);
	if (NULL == item)
		return -EBADF;
		
	if(item->length < count)
		count = item->length;
	
	copy_from_user(item->buffer, buf, count);	
	return count;
}

/**
 * resize_buffer() - resize tree item buffer size
 * @item: tree item
 * @new_size: new buffer size
 */
void resize_buffer(struct hive_ftree_item *item, int new_size)
{
	char *new_buf = kzalloc(sizeof(*new_buf) * new_size, GFP_KERNEL);
	if (NULL == new_buf)
		return;
	
	int i = 0;	
	for(i = 0; i < item->length; i++)
    	new_buf[i] = (item->buffer)[i];
    
    kfree(item->buffer);
    
    item->buffer = new_buf;
    item->length = new_size;
}

/**
 * ioctl_fun() - function for receive commands from user space
 * @file: VFS file opened by a process
 * @cmd: value from user space use like cmd 
 * @arg: value from user space use like arg
 */
long ioctl_fun(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct hive_ftree_item *item = hive_ftree_get(&tree, file);
	if (NULL == item)
		return -EBADF;	
	
	switch(cmd) {
		case IOCTL_BEE_TWERK:
		
		if(item->length < sizeof(magic_phrase))
			resize_buffer(item, sizeof(magic_phrase));
		
		int i = 0;
		for(i = 0; i < sizeof(magic_phrase); i++)
        	(item->buffer)[i] = magic_phrase[i];
		
		MOD_DEBUG(KERN_DEBUG, "Twerk request has been sent");
		
		break;
		case IOCTL_CHG_BUF_SIZE:
			if(arg == 0)
				arg = sizeof(magic_phrase);	
			
			resize_buffer(item, arg);
			MOD_DEBUG(KERN_DEBUG, "New buf size %i", item->length);
		break;
	}
}

// This structure is partially initialized here
// and the rest is initialized by the kernel after call
// to cdev_init()
static struct file_operations hive_fops = {
	.open =    &cdev_open,
	.release = &cdev_release,
	.read =    &cdev_read,
	.write =   &cdev_write,
	.unlocked_ioctl = &ioctl_fun,
	.owner =   THIS_MODULE,
};

/**
 * module_cleanup() - function for free all memory using by module
 */
static void module_cleanup(void)
{
	if (alloc_flags.cdev_added) {
		cdev_del(&hive_cdev);
	}
	if (alloc_flags.dev_created) {
		unregister_chrdev_region(hive_dev, 1);
	}
	delete_tree(&tree);
}


static int __init cdevmod_init(void)
{
	int err = 0;
	
	if (0 == major) {
		// use dynamic allocation (automatic)debugfs_remove_recursive(dirret);
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


	hive_class = class_create(THIS_MODULE, "hivemod_class");
	device_create(hive_class, NULL, hive_dev, NULL, "hivemod");

	dirret = debugfs_create_dir("hivemod", NULL); 
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
	debugfs_remove_recursive(dirret);
	device_destroy(hive_class, hive_dev);
	class_destroy(hive_class);
	module_cleanup();
	MOD_DEBUG(KERN_DEBUG, "All honey reclaimed");
}
 
module_init(cdevmod_init);
module_exit(cdevmod_exit);
