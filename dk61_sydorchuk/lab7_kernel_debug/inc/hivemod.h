#ifndef HIVE_MOD_  
#define HIVE_MOD_

int tree_insert(struct rb_root *root, struct hive_item *data);
static inline void hive_tree_rm(struct hive_item *item);
static inline void hive_tree_rm(struct hive_item *item);
static int cdev_open(struct inode *inode, struct file *file);
static int cdev_release(struct inode *inode, struct file *file);
static void module_cleanup(void);
static int __init cdevmod_init(void);
static void __exit cdevmod_exit(void);

#endif