/**	
 * based on:
 * 1. https://github.com/kpi-keoa/kpi-embedded-linux-course/
 * blob/master/demo/lab1/firstmod.c
 * by thodnev
 */

#include <linux/module.h>	 // required by all modules
#include <linux/kernel.h>	 // required for sysinfo
#include <linux/init.h>		 // used by module_init, module_exit macros
#include <linux/kthread.h>
#include <linux/list.h>
#include <asm/atomic.h>
#include <linux/slab.h> 	 //kmalloc declared here 

MODULE_DESCRIPTION("Module with: kthread, atomic, linked_list, lock, unlock");
MODULE_AUTHOR("thodnev ; bramory"); 
MODULE_VERSION("0.1");
MODULE_LICENSE("DualMIT/GPL");	 //it affects the kernel behavior

struct task_struct **threads;
static int N_THREADS = 10;
static int increment = 2;
static int N_TIMES = 500000;
static int *cnt; 			 

module_param(N_THREADS, int, 0); //limited by free RAM
module_param(increment, int, 0);
module_param(N_TIMES, int, 0);

MODULE_PARM_DESC(N_THREADS, "Number of kthreads to create");
MODULE_PARM_DESC(increment, "Step for increasing global cnt");
MODULE_PARM_DESC(N_TIMES, "Value for the the thread to count");

#define LOCKED 0
#define UNLOCKED 1

atomic_t *var_lock = NULL;

/* Interface for working with atomic_t variables*/
/* return UNLOCKED spinlock */
static void* create_lock(void)
{
	atomic_t *ptr_lock = kmalloc(sizeof *ptr_lock, GFP_ATOMIC);
	arch_atomic_set(ptr_lock, UNLOCKED);
	return ptr_lock;
}

static void lock(atomic_t *lock)
{
	while (LOCKED == arch_atomic_cmpxchg(lock, UNLOCKED, LOCKED));
}

static void unlock(atomic_t *arg)
{
	arch_atomic_set(arg, UNLOCKED);
}


/* Working with lists */
struct list_node {
    struct list_head entry;
    int data;
};

LIST_HEAD(list_with_nodes);

/* Thread handler function */
int counter(void *data)
{
	lock(var_lock);

		for (int i = 0; i < N_TIMES; ++i) {
			*cnt += increment; 
		}

		struct list_node *current_node = kmalloc(sizeof(*current_node), GFP_KERNEL);
		if (NULL == current_node) {
			printk(KERN_ERR "Can't allocate a memory for the list node\n");
			return -4;
		}

		current_node->data = *cnt;
		list_add(&current_node->entry, &list_with_nodes);

	unlock(var_lock);
	return 0;
}

/* Initialization part for this module */
static int __init firstmod_init(void)
{

	var_lock = create_lock();
	if (NULL == var_lock) {
		printk(KERN_ERR "Can't allocate a memory for the spinlock\n");
		return -1;
	}

	cnt = kmalloc(sizeof cnt, GFP_KERNEL);
	if (NULL == cnt)
		return -2;

	// initialization
	*cnt = 0;

	printk(KERN_INFO "Counter = %d\n", *cnt);
	threads = kmalloc(sizeof (*threads) * N_THREADS, GFP_KERNEL);
	if (NULL == threads) {
		printk(KERN_ERR "Can't allocate a memory for the threads\n");
	return -3;
	}

	for(int i = 0; i < N_THREADS; ++i) {
		threads[i] = kthread_run(&counter, (void*) i, "thread_%i", i);
	}
	return 0;
}
 

/* Actions to do before unloading this module from the kernel */
static void __exit firstmod_exit(void)
{
	printk(KERN_INFO "Global_Counter = %i\n", *cnt);
	kfree(threads);
	kfree(var_lock);

	struct list_head *it, *tmp;
	struct list_node *node_to_del;
	list_for_each_safe(it, tmp, &list_with_nodes) {
		int list_val = (container_of(it, struct list_node, entry))->data;
		printk(KERN_INFO "List val is %i\n", list_val);
		node_to_del = list_entry(it, struct list_node, entry);
		list_del(it);
		kfree(node_to_del);
	}

	kfree(cnt);	
}

module_init(firstmod_init);
module_exit(firstmod_exit);
	