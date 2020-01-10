/**
 * based on https://bit.ly/2kLBtD9 by thodnev
 * 'lock()' and 'unlock()' functions were implemented together with
 * Maxim Salim
 */
#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/list.h>		// used by linked list
#include <linux/slab.h>		// used by kmalloc()
#include <linux/kthread.h>	// used by kernel threads
#include <asm/atomic.h>		// used by atomic instruction

MODULE_DESCRIPTION("Example linux linked list with atomic operation");
MODULE_AUTHOR("MaksHolub");
MODULE_VERSION("1.0");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

/**
 * struct k_list - struct of element kernel linked list.
 * @test_list: 	list_head type, which neccessary to provide kernel 
 *		linked list mechanism 				
 * @count_val: 	Current count value of node.
 * @thread_cnt: used to identify of node
 *          
 * This struct neccessary to work with kernel linked list
 * It contains a special variable test_list, 
 * which is an instance of a struct kernel linked list.
 */

struct k_list {

	struct list_head test_list;
	int count_val;
	int thread_cnt;

};

/**
 * LIST_HEAD provide initialization of kernel linked list
 */
LIST_HEAD(head_list); 

static int M = 2;            
module_param(M, int, 0);
MODULE_PARM_DESC(M, "Numbers of threads");

static int N = 10000000;           
module_param(N, int, 0);
MODULE_PARM_DESC(N, "Quantity of iteration count");

struct k_list *data;

/**
 * @count: pointer, which contains address on variable of type int.
 *		Global variable, which will be incremented in threads.    
 */

static int *count; 

/**
 * @local_mutex: pointer, which contains address on variable of type atomic_t.
 *		 In this module this variable was used for emulations of mutex.    
 */

static atomic_t *local_mutex;

/**
 * create_lock - memory allocation and initialization of the lock variable.    
 */

static atomic_t *create_lock(void)
{
	atomic_t *ptr_lock = kmalloc(sizeof(*ptr_lock), GFP_KERNEL);

	if (!ptr_lock) {

		printk(KERN_ERR "Allocation error (ptr_lock)");	
		goto errn;
	}

	atomic_set(ptr_lock, 0);
	return ptr_lock;

	errn:
   		kfree(ptr_lock);
   		return -ENOMEM;

}

/**
 * lock - function of increment global counter. 
 * @argument: pointer, which contains address of lock variable.   
 */

static void lock(atomic_t *argument)
{
	while(atomic_cmpxchg(argument, 0, 1));
}

/**
 * unlock - function of increment global counter. 
 * @argument: pointer, which contains address of lock variable.   
 */

static void unlock(atomic_t *argument)
{
	atomic_set(argument, 0);
}

/**
 * thread_func - function of increment global counter. 
 * @argument: pointer, which contains address of thread argument.   
 */

static int thread_func(void *argument)
{
	lock(local_mutex);

  	if (N > 0) {
    		for (int i = 1; i <= N; i++)
    			(*(int *)argument)++;
  	}

	data = kmalloc(sizeof(struct k_list), GFP_KERNEL);
	
	if (!data) {

		printk(KERN_ERR "Allocation error (data)");
		goto errn;	
	
	}
	
	data->count_val = *(int *)argument;
	data->thread_cnt = (*(int *)argument)/N;

	list_add(&data->test_list, &head_list);
	unlock(local_mutex);

	return 0;
	errn:
   		kfree(data);
   		return -ENOMEM;
}

/**
 * listtest_show_list - print the content of kernel linked list.    
 */

static void listtest_show_list(void)
{

        struct list_head *listptr;
        struct k_list *entry;
		printk(KERN_ALERT "Show_list\n");

		list_for_each(listptr, &head_list) 
		{
		        entry = list_entry(listptr, struct k_list, test_list);
		    
			printk(KERN_INFO "Thread number %d, count to =  %d ", \
					entry->thread_cnt, entry->count_val);
		}
}

/**
 * delete_list - delete the content of kernel linked list.    
 */

void delete_list(void)
{
	struct list_head *listptr, *tmp;
        struct k_list *entry;
		list_for_each_safe(listptr, tmp, &head_list)
		{

			entry = list_entry(listptr, struct k_list, test_list);
			
			printk(KERN_NOTICE "Deleting #%d!\n", entry->thread_cnt);

			list_del(listptr);
			kfree(entry);		
		}
}

static int __init create_list_init(void) 
{

	count = kmalloc(sizeof(int), GFP_KERNEL);

	if (!count) {

		printk(KERN_ERR "Allocation error (count)");	
		goto mem_error;
	}	

	*count = 0;
	
	local_mutex = create_lock();
	
	/** 
	 * thread_t - Array of pointers to thread structures 
	 */

	struct task_struct **thread_t = kmalloc(M * sizeof(**thread_t), GFP_KERNEL);

	if (!thread_t) {

		printk(KERN_ERR "Allocation error (thread_t)");	
		goto errn;
	}

	for (int i = 0; i < M; i++) {

		thread_t[i] = kthread_run(thread_func, (void*)count, "mykthread_%d", i);	

	}
	kfree(thread_t);	

	return 0;

	errn:
		kfree(count);
   		kfree(thread_t);
   		return -ENOMEM;

	mem_error:
		kfree(count);
   		return -ENOMEM;		

}
	

static void __exit create_list_exit(void) 
{
	printk(KERN_ALERT "Result = %d\n", *count);	
	listtest_show_list();
	delete_list();
	kfree(data);
	kfree(local_mutex);
	kfree(count);
	printk(KERN_ALERT "Module exit\n");	
}

module_init(create_list_init);
module_exit(create_list_exit); 

