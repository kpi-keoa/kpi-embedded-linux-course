#include <linux/module.h>	
#include <linux/kernel.h>	
#include <linux/init.h>		
#include <linux/list.h>		
#include <linux/slab.h>		
#include <linux/kthread.h>
#include<linux/delay.h>	
#include <linux/spinlock.h>	

MODULE_DESCRIPTION("Linux linked list with atomic operation");
MODULE_AUTHOR("Anton Kotsiubailo");
MODULE_VERSION("1.0");
MODULE_LICENSE("Dual MIT/GPL");		


struct k_list {

	struct list_head test_list;
	int count_val;
	int thread_cnt;

};

LIST_HEAD(head_list); 

static int thread_num = 0;            
module_param(thread_num, int, 0);
MODULE_PARM_DESC(thread_num, "Numbers of threads");

static int thread_delay = 0;
module_param(thread_delay, int, 0);
MODULE_PARM_DESC(thread_delay,"delay between threads");

static int thread_inccnt = 0;           
module_param(thread_inccnt, int, 0);
MODULE_PARM_DESC(thread_inccnt, "Quantity of iteration count");

struct k_list *data;



static int *global_var; 



static atomic_t *local_mutex;



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




static void lock(atomic_t *argument)
{
	while(atomic_cmpxchg(argument, 0, 1));
}




static void unlock(atomic_t *argument)
{
	atomic_set(argument, 0);
}



static int thread_func(void *argument)
{
	spin_lock(local_mutex);

  	if (thread_inccnt > 0) {
    		for (int i = 1; i <= thread_inccnt; i++) {
    			(*(int *)argument)++;
                        msleep(thread_delay);
                        
                }
  	}

	data = kmalloc(sizeof(struct k_list), GFP_KERNEL);
	
	if (!data) {

		printk(KERN_ERR "Allocation error (data)");
		goto errn;	
	
	}
	
	data->count_val = *(int *)argument;
	data->thread_cnt = (*(int *)argument)/thread_inccnt;

	list_add(&data->test_list, &head_list);
	spin_unlock(local_mutex);

	return 0;
	errn:
   		kfree(data);
   		return -ENOMEM;
}



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

        if(thread_num < 0) {
		printk(KERN_ERR "Invalid thread number <= 0 :(\n");
		goto final;
	}

	if(thread_delay < 0) {
		printk(KERN_ERR "Invalid thread delay <= 0 :(\n");
		goto final;
	}

	if(thread_inccnt < 0) {
		printk(KERN_ERR "Invalid quantity of iteration count  <= 0 :(\n");
		goto final;
	}

	count = kmalloc(sizeof(int), GFP_KERNEL);

	if (!count) {

		printk(KERN_ERR "Allocation error (count)");	
		goto mem_error;
	}	

	*count = 0;
	
	local_mutex = create_lock();
	
	

	struct task_struct **thread_t = kmalloc(thread_num * sizeof(**thread_t), GFP_KERNEL);

	if (!thread_t) {

		printk(KERN_ERR "Allocation error (thread_t)");	
		goto errn;
	}

	for (int i = 0; i < thread_num; i++) {

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

        final:	
                return -EINVAL;

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
