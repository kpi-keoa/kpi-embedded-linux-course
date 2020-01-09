/**

*	lock, unlock, create_lock functions were implemented by 
*	Bramory - /github.com/Bramory/ 

*/

#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/interrupt.h>
#include <linux/kthread.h>	// used by kernel threads
#include <linux/list.h>
#include <linux/slab.h>		// used for memory allocating
#include <asm/atomic.h>		// used by lock() and unlock() functions

#define UNLOCKED 1
#define LOCKED 0

MODULE_DESCRIPTION("Simple kernel module");
MODULE_AUTHOR("ra7e");
MODULE_VERSION("0.5");
MODULE_LICENSE("Dual MIT/GPL");		

static int n_of_threads = 10;
static int max_number = 1000000;
static int iter = 1;


module_param (n_of_threads, int, 0000);
module_param (max_number, int, 0000);
module_param (iter, int, 0000);

struct task_struct **threads;

static int *glob_cnt;

atomic_t *p = NULL;

struct my_list {
	struct list_head list;
	int number;
};

LIST_HEAD(first_node_LIST);

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

int thread_func(void *iterator)
{
	lock(p);
	
	for(int i = 0; i < max_number; i++)
	{
		*glob_cnt += *(int*)iterator;
	}

	struct my_list *this_node = kmalloc(sizeof(*this_node), GFP_KERNEL);
	
	if(NULL == this_node){
		printk(KERN_ERR"Error");
		return -1;
	}

	printk(KERN_INFO "Counter %i", *glob_cnt);
	
	this_node->number = *glob_cnt;

	list_add(&this_node->list, &first_node_LIST);
	
	unlock(p);
	
	return 0;
		
}	

static void print_list(struct list_head *for_print)
{
	struct list_head *prt;
	struct my_list *data;
	
	list_for_each(prt, for_print)
	{
		data = list_entry(prt, struct my_list, list);	
		printk(KERN_INFO "value %d", data->number);
	}
}

static void delete_list(struct list_head *for_del)
{
	struct list_head *ptr, *tmp;
		list_for_each_safe(ptr, tmp, for_del)
		{
			list_del(ptr); 
		}
	printk(KERN_INFO "Threads deleted\n");
}

static int __init firstmod_init(void)
{
	p = create_lock();

	printk(KERN_INFO "Entry point");
	
	printk(KERN_INFO "Thread creating ...\n");
	
	glob_cnt = kmalloc(sizeof glob_cnt, GFP_KERNEL);
	if(glob_cnt == NULL)
		return -2;

	threads = kmalloc(sizeof(*threads) * n_of_threads, GFP_KERNEL);
	
	if(NULL == threads){
		printk(KERN_ERR "Memory allocating error");
		return -2;
	}

	int *iterator = &iter;

	*glob_cnt = 0;

	for(int i = 0; i <= n_of_threads; i++)
	{	
		threads[i] = kthread_run(&thread_func, (void*)iterator, "Thread %i", i);//(void*) i
	}

	return 0;	
}
 
static void __exit firstmod_exit(void)
{	
	printk(KERN_INFO "PRINTING...\n");
	print_list(&first_node_LIST);
	printk(KERN_INFO "DELETING...\n");
	delete_list(&first_node_LIST);
	printk(KERN_INFO "Exit");
	kfree(threads);
	kfree(glob_cnt);
	kfree(p);	
	printk(KERN_INFO "Exit done"); 
	
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);

