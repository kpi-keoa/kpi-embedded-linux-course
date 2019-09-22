/* Based on https://bit.ly/2kLBtD9 made by thodnev.
 * This code was implemented by:  
 *	-Olexander Ostrianko
 *	-Maxim Salim
 *	-Dana Dovzhenko
 * Task with global variable, list, threads and memory allocating was 
 * acomplished by Olexander Ostrianko and Dana Dovzhenko. 
 * 'lock()' and 'unlock()' functions were implemented by Maxim Salim and also 
 * minor fixes added by Maxim Holub. This functions used for threads 
 * syncronization.
 */

#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/interrupt.h>
#include <linux/kthread.h>	// used by kernel threads
#include <linux/list.h>
#include <linux/slab.h>		// used for memory allocating
#include <asm/atomic.h>		// used by lock() and unlock() functions


MODULE_DESCRIPTION("Basic module demo: kthread, list, atomic");
MODULE_AUTHOR("AlexOstrianko; maksimo0; DovzhenkoD");
MODULE_VERSION("0.3");
MODULE_LICENSE("Dual MIT/GPL");	

static int number_of_cycles = 1000;
static int number_of_threads = 5;

int *count;

module_param(number_of_cycles, int, 0);
module_param(number_of_threads, int, 0);

MODULE_PARM_DESC(number_of_cycles, "The number to which threads are incremented");
MODULE_PARM_DESC(number_of_threads, "Number of threads to create");


typedef struct struct_for_saving_results_n {
	struct list_head list;
	int num;
}struct_for_saving_results;
	
struct task_struct **threads_pointer;
struct_for_saving_results *struct_pointer;
struct_for_saving_results main_struct;
atomic_t *lock_key;

static atomic_t *get_new_lock(void){
	atomic_t *ptr = (atomic_t *)kmalloc(sizeof(atomic_t), GFP_ATOMIC);
	arch_atomic_set(ptr, 0);
	return ptr;
}

static void lock(atomic_t *arg){
	while(arch_atomic_xchg(arg, 1)){
		schedule(); 	
	}
}

static void unlock(atomic_t *arg){
	arch_atomic_set(arg, 0);
}

static void delete_lock(atomic_t *arg){
	kfree(arg);
}


int thread_function(void *data)
{
	int i;
	for(i = 0; i < number_of_cycles; i++){
		lock(lock_key);			//ucomment if necessary
		*(int *)data = *(int *)data + 1;
		unlock(lock_key);		//uncomment if necessary
		schedule();
	}
	struct_pointer = (struct_for_saving_results *)kmalloc(
		sizeof(struct_for_saving_results), GFP_KERNEL);
	if(!struct_pointer)
		goto Struct_Error;
	struct_pointer->num = *(int *)data;	//protection is not needed here, since other functions do not yet have access to this variable
	list_add(&struct_pointer->list, &main_struct.list);
	do_exit(1);
	Struct_Error: printk(KERN_ERR "Kmalloc doesnt allocate memory!\n");
	do_exit(1);
}


static int __init mod_init( void ) { 
	
	INIT_LIST_HEAD(&main_struct.list);
	
	lock_key = get_new_lock();
	if(!lock_key) 
		goto Lock_Error;	
	count = (int *)kmalloc(sizeof(int), GFP_KERNEL);
	if (!count) 
		goto Count_Error;
	*count = 0;
	threads_pointer = (struct task_struct **)kmalloc(
		sizeof(struct task_struct *) * number_of_threads, GFP_KERNEL);	
	if (!threads_pointer) 
		goto Threads_Error;	
	int i;
	for(i = 0; i < number_of_threads; i++){
		threads_pointer[i] = kthread_run(thread_function, (void *)count, 
			"thread_%d", i);
	}
	return 0; 
	
	Threads_Error: kfree(count);
	Count_Error: delete_lock(lock_key);
	Lock_Error: printk(KERN_ERR "Kmalloc doesn`t allocate memory!\n"); 
	return ENOMEM;
}

static void __exit mod_exit( void ) { 
	
	printk(KERN_NOTICE "Counter value is #%d!\n", *count);
	struct list_head *head_pointer;
	struct list_head *temp;
	list_for_each_safe(head_pointer, temp, &(main_struct.list)){
		struct_pointer = list_entry(head_pointer, 
			struct_for_saving_results, list);
		printk(KERN_NOTICE "Deleting list element #%d!\n",
			struct_pointer->num);
		list_del(head_pointer);
		kfree(struct_pointer);		
	}
	if(count)
		kfree(count);
	if(lock_key)
		delete_lock(lock_key);
	printk(KERN_NOTICE "Module unloaded!\n" ); 
}


module_init(mod_init);
module_exit(mod_exit);

