#include <linux/module.h>	// required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <asm/atomic.h>

const int LOCK = 1;
const int UNLOCK = 0;

const int LOCKER_NOT_CREATED = -1;
const int NOT_ENAUGHT_MEMORY = -2;

MODULE_DESCRIPTION("Kernel threads");
MODULE_AUTHOR("olegovichh22");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

int num_of_threads = 1;
module_param(num_of_threads, int, 0000);
MODULE_PARM_DESC(num_of_threads, "Num of threads");

int num_of_increments = 10;
module_param(num_of_increments, int, 0000);
MODULE_PARM_DESC(num_of_increments, "Num of increments for every thread");

static struct task_struct **thread = NULL;
static int *inc_var = NULL;


static LIST_HEAD(list_of_ints);
struct int_node {
	struct list_head list;
	int data;
};

void print_and_erase_list(struct list_head *print_list)
{
	if(NULL == print_list) {
    	printk(KERN_ERR "Cant print NULL list\n");
      	return;
    }
	
	struct list_head *current_pos = NULL;
	struct list_head *tmp = NULL;
	struct int_node * current_node = NULL;
	list_for_each_safe(current_pos, tmp, print_list) {
         
         current_node = list_entry(current_pos, struct int_node, list);
         
         printk ("List element = %d\n" , current_node->data);
         
         list_del(current_pos);
         kfree(current_node);
    }
}

static atomic_t *lock_list;
static atomic_t *lock_increment;

atomic_t *create_locker(void)
{
	atomic_t *var = kmalloc(sizeof(atomic_t), GFP_ATOMIC);
	if(NULL == var) {
		printk(KERN_ERR "LOCKER NOT CREATED - HAVENT ENAUGHT MEMORY\n");
		return NULL;
	}
	
	atomic_set(var, UNLOCK);
	
	return var;
}

void delete_locker(atomic_t *locker)
{
	if(NULL == locker)
	kfree(locker);
} 


//how to work atomic_cmpxchg 
//old = *p
//*p = (old == UNLOCK) ? LOCK : old
//return old;
void lock(atomic_t *var)
{		
	while(atomic_cmpxchg(var, UNLOCK, LOCK));
}

void unlock(atomic_t *var)
{
	atomic_set(var, UNLOCK);
}

int thread_fun(void *arg)
{
	int *inc_var = (int *)arg;
	
	int i = 0;
	
	lock(lock_increment);
	for(i = 0; i < num_of_increments; i++) {
		(*inc_var)++;
	}
	int value_for_write = (*inc_var);
	unlock(lock_increment);
	
	struct int_node * node = kmalloc(sizeof(struct int_node), GFP_KERNEL);
	if(NULL == node) {
		printk(KERN_ERR "HAVENT ENOUGH MEMORY FOR LIST NODE\n");
		print_and_erase_list(&list_of_ints);
		goto end;
	}
	
	node->data = value_for_write;
	lock(lock_list);
	list_add(&node->list, &list_of_ints);
	unlock(lock_list);
	
	end:
	
	do_exit(0);
}


static int __init kernel_thread_init(void)
{	
	printk(KERN_INFO "Num of threads  	%i\n", num_of_threads);
	printk(KERN_INFO "Num of increments %i\n", num_of_increments);
	
	lock_list = create_locker();
	if(NULL == lock_list)
	return LOCKER_NOT_CREATED;
	
	lock_increment = create_locker();
	if(NULL == lock_increment)
	return LOCKER_NOT_CREATED;
	
	inc_var = kmalloc(sizeof(int), GFP_KERNEL);
	if(NULL == inc_var) {
		printk(KERN_ERR "NOT ENAUGHT MEMORY\n");
		return NOT_ENAUGHT_MEMORY;	
	}
	
	(*inc_var) = 0;
	
	thread = kmalloc(sizeof(struct task_struct **)*num_of_threads, GFP_KERNEL);
	
	if(NULL == thread) {
		printk(KERN_ERR "NOT ENAUGHT MEMORY\n");
		return NOT_ENAUGHT_MEMORY;	
	}
	
	int i=0;
	for(i = 0; i < num_of_threads; i++) {
		thread[i] = kthread_run(thread_fun, (void *)inc_var, "my_thread");
		if(thread[i] == ERR_PTR(-ENOMEM)) {
			printk(KERN_ERR "Thread %i NOT started\n", i);	
		}
		else {
			printk(KERN_INFO "Thread %i started\n", i);
		}
		
	}

	printk(KERN_INFO "Main thread end\n");
	
	return 0;
}
 
static void __exit kernel_thread_exit(void)
{
	printk(KERN_INFO "Closing the module\n");
	
	print_and_erase_list(&list_of_ints);
	
	if (NULL != thread) 
	kfree(thread);
	
	if(NULL == inc_var)
	kfree(inc_var);
	
	delete_locker(lock_list);
	delete_locker(lock_increment);
	
	printk(KERN_INFO "See you later aligator!\n");
}
 
module_init(kernel_thread_init);
module_exit(kernel_thread_exit);
