#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>       // to allow registration of device at /dev
#include <linux/uaccess.h>      // for get_user and put_user functions
#include <linux/types.h>        // for using the size_t type
#include <linux/rbtree.h>

MODULE_DESCRIPTION("red-black tree Kernel API demo");
MODULE_AUTHOR("max shvayuk");
MODULE_VERSION("0.228");
MODULE_LICENSE("Dual MIT/GPL");

struct rb_root the_root = RB_ROOT;

/* the rbtrre functions src is https://lwn.net/Articles/184495/ */
struct tree_item {
    int val;
    struct rb_node node;
};

struct tree_item *my_rb_search(struct rb_root *root, int value)
{        
        struct rb_node *node = root->rb_node;  /* top of the tree */

        while (node)
        {
                struct tree_item *tmp = rb_entry(node, struct tree_item, node);

                if (tmp->val > value)
                        node = node->rb_left;
                else if (tmp->val < value)
                        node = node->rb_right;
                else {
                        printk(KERN_ERR "collision at the %i val\n", value);
                        return tmp;  /* Found it */
                }
        }
        return NULL;
}

void my_rb_insert(struct rb_root *root, struct tree_item *new)
{        
        struct rb_node **link = &(root->rb_node);
        struct rb_node *parent = NULL;
        int value = new->val;
        
        /* Go to the bottom of the tree */
        while (*link)
        {
                parent = *link;
                struct tree_item *tmp = rb_entry(parent, struct tree_item, node);

                if (tmp->val > value)
                        link = &((*link)->rb_left);
                else if (tmp->val < value)
                        link = &((*link)->rb_right);
                else {
                        printk(KERN_ERR "insert error\n");
                        return;
                }
        }

        /* Put the new node there */
        rb_link_node(&(new->node), parent, link);
        rb_insert_color(&(new->node), root);
}

void rb_print_node(struct rb_node *node, int parent_val, const char *node_type)
{
        int val = (rb_entry(node, struct tree_item, node))->val;
        printk(KERN_INFO "%i->%s %i \n",parent_val, node_type, val);
        if (NULL != node->rb_left)
                rb_print_node(node->rb_left, val, "left: ");
        if (NULL != node->rb_right) 
                rb_print_node(node->rb_right, val, "right: ");
}

void rb_print_tree(struct rb_root *root)
{
        struct rb_node *node = root->rb_node;  /* top of the tree */
        rb_print_node(node, 999999, "root: ");
}

// void rb_delete_node(struct rb_node *node, struct rb_root *root)
// {
//         if (NULL == node)
//                 return;
//         
//         printk(KERN_INFO "try to delete some node\n");
//         int val = (rb_entry(node, struct tree_item, node))->val;
//         
//         if (NULL != node->rb_left) {
//                 rb_delete_node(node->rb_left, root);
//                 return;
//         }
//         if (NULL != node->rb_right) {
//                 rb_delete_node(node->rb_right, root);
//                 return;
//         }
//         
//         printk(KERN_INFO "node to be deleted: %i\n", val);
//         rb_erase(node, root); /* delete node from three */
//         kfree(rb_entry(node, struct tree_item, node));        
//         
//         printk(KERN_INFO "node %i deleted\n\n", val);
//         rb_print_tree(root);
// }
// 
// void rb_delete_tree(struct rb_root *root)
// {
//         struct rb_node *node = root->rb_node;  /* top of the tree */
//         while (NULL != the_root.rb_node->rb_left)
//                 rb_delete_node(node->rb_left, &the_root);
//         while (NULL != the_root.rb_node->rb_right)
//                 rb_delete_node(node->rb_right, &the_root);
//         
//         kfree(node);
//         printk(KERN_INFO "list deleted\n");
// }

int rb_delete_tree(struct rb_root *root)
{
        int cnt = 0;
        struct rb_node *node_it = rb_first(root);
        struct rb_node *node_tmp = NULL;
        while (node_it) {
                node_tmp = rb_next(node_it);
                int val = rb_entry(node_it, struct tree_item, node)->val;
//                 printk(KERN_INFO "node to be deleted: %i\n", val);
                rb_erase(node_it, root);
                kfree(rb_entry(node_it, struct tree_item, node));
//                 printk(KERN_INFO "node has been deleted: %i\n", val);
                node_it = node_tmp;
                cnt++;
        }
        return cnt;
}

static int __init cdevmod_init(void)
{     
	for (int i = 1; i < 21; i++) {
                struct tree_item *new_item = kmalloc(sizeof *new_item, GFP_KERNEL);
                if (NULL == new_item) {
                        printk(KERN_ERR "can't allocate the memory for the new item\n");
                        continue;
                }
                new_item->val = i;
                
                if (NULL == my_rb_search(&the_root, i))
                        my_rb_insert(&the_root, new_item);
                
                printk(KERN_INFO "node %i created\n", i);
        }
        
        rb_print_tree(&the_root);
        
        int num_deleted_nodes =  rb_delete_tree(&the_root);
        printk(KERN_INFO "deleted %i nodes\n", num_deleted_nodes);
	return 0;
}
 
static void __exit cdevmod_exit(void)
{
	printk(KERN_INFO "exit\n");
}
 
module_init(cdevmod_init);
module_exit(cdevmod_exit);
