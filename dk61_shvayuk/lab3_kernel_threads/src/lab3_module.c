
#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/kthread.h>  	// for threads
#include <linux/timer.h>
#include <linux/slab.h>		// for kmalloc()
#include <linux/list.h>		// for linked list
#include <asm/atomic.h>		// for atomic "compare-exchange" operation
#include <linux/types.h>	// for atomic_t type declaration

#define THREADS_RETVAL		(0)
#define SPINLOCK_IS_LOCKED	(1)
#define SPINLOCK_IS_UNLOCKED	(0)

MODULE_DESCRIPTION("Module works with Linux kernel threads, linked list, "
		    "and atomic operations");
MODULE_AUTHOR("max_shvayuk");
MODULE_VERSION("0.228");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

int num_threads = 0;
int thread_inc_iterations = 0;
module_param(num_threads, int, 0);
module_param(thread_inc_iterations, int, 0);

MODULE_PARM_DESC(num_threads, "Number of kernel threads");
MODULE_PARM_DESC(thread_inc_iterations, "The number of a global variable "
                 "iterations, each thread has to perform");

struct task_struct **threads;
LIST_HEAD(list_for_variables);
atomic_t *spinlock_var_handler;
atomic_t *spinlock_list_handler;
int global_var = 0;

void spinlock_lock(atomic_t *ptr);
void spinlock_unlock(atomic_t *ptr);
int spinlock_init(atomic_t **ptr);
int thread_handler (void *data);

int spinlock_init(atomic_t **ptr)
{
	*ptr = kmalloc(sizeof(*(*ptr)), GFP_ATOMIC);
	if (NULL == *ptr) {
		printk(KERN_ERR "Can't allocate memory for spinlock"
					", aborting\n");
		return -1;
	}
	spinlock_unlock(*ptr);
	
	return 0;
}

void spinlock_lock(atomic_t *ptr)
{
	unsigned long retval = 0;
	do
		retval = atomic_cmpxchg(ptr, SPINLOCK_IS_UNLOCKED, 
					SPINLOCK_IS_LOCKED);
	while (SPINLOCK_IS_LOCKED == retval);
}

void spinlock_unlock(atomic_t *ptr)
{
	atomic_set(ptr, SPINLOCK_IS_UNLOCKED);
}

struct list_node
{
	struct list_head next;
	int data;
};

int thread_handler (void *data)
{
	printk(KERN_INFO "Hello from thread\n");
	struct list_node *node_ptr = kmalloc(sizeof(*node_ptr), GFP_KERNEL);
	if (NULL == node_ptr) {
		printk(KERN_ERR "Can't allocate memory for list node\n");
		goto THREAD_END_LABEL;
	}
	
	spinlock_lock(spinlock_var_handler);
	for (int i = 0; i < thread_inc_iterations; i++)
		(*((int *)data))++;
	node_ptr->data = *((int*)data);
	spinlock_unlock(spinlock_var_handler);

	spinlock_lock(spinlock_list_handler);
	list_add(&(node_ptr->next), &list_for_variables);
	spinlock_unlock(spinlock_list_handler);
	
	THREAD_END_LABEL:
	while (1) {
		if (kthread_should_stop())
			do_exit(THREADS_RETVAL);
		schedule();
	}
}

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello from module");
	
	/* Create spinlocks */
	if (0 != spinlock_init(&spinlock_var_handler))
		return -2;
	if (0 != spinlock_init(&spinlock_list_handler))
		return -3;

	/* Allocate memory for threads handlers */
	threads = kmalloc (num_threads * sizeof(*threads), GFP_KERNEL);
	if (NULL == threads) {
		printk(KERN_ERR "Can't allocate memory for thread's handlers"
		                 ", aborting\n");
		return -1;
	}
	
	/* Create threads */
	for (int i = 0; i < num_threads; i++) {
		threads[i] = kthread_create(thread_handler, &global_var, 
					    "test_thread");
		if ((ERR_PTR(-ENOMEM)) == threads[i]) {
			printk(KERN_ERR "Can't allocate memory for thread %i\n",
			       i);
			for (i=i; i != 0; i--)
				kfree(threads[i]);
			return -4;
		}
	}
	
	/* Run threads */
	for (int i = 0; i < num_threads; i++)
		wake_up_process(threads[i]);
	
	printk(KERN_INFO "Threads started\n");
	
	return 0;
}
 
static void __exit firstmod_exit(void)
{
	if (NULL != threads) {
		for (int i = 0; i < num_threads; i++) {
			int error_check = kthread_stop(threads[i]);
			if (THREADS_RETVAL == error_check)
				printk(KERN_INFO "Thread stopped\n");
			else
				printk(KERN_ERR "Some error occured while "
				       "trying to stop the thread\n");
		}
	}
	
	struct list_head *it;
	list_for_each(it, &list_for_variables) {
		int list_val = (container_of(it, struct list_node, next))->data;
		printk(KERN_INFO "List val is %i\n", list_val);
	}
	struct list_head *tmp;
	list_for_each_safe(it, tmp, &list_for_variables) {
		list_del(it);
	}
	
	/* Free the spinlocks's memory */
	kfree(spinlock_var_handler);
	kfree(spinlock_list_handler);
	
	printk(KERN_INFO "Final val is %i\n", global_var);
	
	printk(KERN_INFO "Long live the Kernel!\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);
