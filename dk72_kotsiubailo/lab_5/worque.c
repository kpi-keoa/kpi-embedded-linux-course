#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/interrupt.h>	// used by kernel interrupt
#include <linux/kthread.h>	// used by kernel threads
#include <linux/list.h>		// used by kernel list
#include <linux/slab.h>		// used for memory allocating
#include <asm/atomic.h>		// used by lock() and unlock() functions
#include <linux/timer.h>	// used by kernel timer
#include <linux/workqueue.h>	// used by kernel workqueue
#include <linux/semaphore.h>	// used by semaphore
#include <linux/delay.h>	// used by delay work


*/
MODULE_DESCRIPTION("Simple operation with workqueue and timers");
MODULE_AUTHOR("Anton Kotsiubailo");
MODULE_VERSION("4.0");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior


struct k_list {
	struct list_head test_list;
	struct rcu_head rcu;
	long long count_val;
};


struct threads_flags {
	u32 first_active:1;
	u32 second_active:1;
};

struct mod_allocs {
        u32 

};

static struct flags flags_stat = {0};

static DEFINE_SPINLOCK(rcu_lock);
struct timer_list timer;


LIST_HEAD(head_first_list);
LIST_HEAD(head_second_list); 

const int NUM_OF_THREADS = 2;

struct delayed_work work;

struct task_struct **thread_t;



static void listtest_show_list(struct list_head *a_list)
{

        struct list_head *listptr;
        struct k_list *entry;
	printk(KERN_ALERT "Show_list\n");

	rcu_read_lock();

	list_for_each_entry_rcu(entry, a_list, test_list) {
		printk(KERN_INFO "Jiffies =  %lld ", entry->count_val);
	}

	rcu_read_unlock();
}


static int first_thread_func(void *argument)
{
  
	while (flags_stat.first_thread_active) {

		 schedule();
   	}


	listtest_show_list(&head_first_list);

	return 0;
}


static int second_thread_func(void *argument)
{
	
	while (!kthread_should_stop()) {

		 schedule();
   	}

	listtest_show_list(&head_second_list);
	flags_stat.second_thread_active = 0;

	return 0;

}


void work_handler(struct work_struct *arg)
{
	
	if ((jiffies%11) == 0) {
		
		kthread_stop(thread_t[1]);
		flags_stat.second_thread_active = 0;		

	} else {

		struct k_list *data;
		if (!(data = kmalloc(sizeof(struct k_list), GFP_KERNEL))) {
			printk(KERN_ERR "Allocation error of node");
		} else {
			data->count_val = jiffies;
			spin_lock(&rcu_lock);
			list_add_rcu(&data->test_list, &head_second_list);
			spin_unlock(&rcu_lock);
			schedule_delayed_work(&work, 17);
		}
	}
}


void timer_handler(struct timer_list *tim)
{
		
	if ((jiffies%11) == 0) {
		
		flags_stat.first_thread_active = 0;			

	} else {

		struct k_list *data;
		if (!(data = kmalloc(sizeof(struct k_list), GFP_ATOMIC))) {
			printk(KERN_ERR "Allocation error of node");
		} else {
			data->count_val = jiffies;
			spin_lock(&rcu_lock);
			list_add_rcu(&data->test_list, &head_first_list);
			spin_unlock(&rcu_lock);
			mod_timer(&timer, jiffies + 17);
		}

	}
}


void delete_list(struct list_head *a_list)
{

	if (NULL == a_list)
		return;

	struct k_list *entry;
        struct list_head *del_node, *tmp;

	spin_lock(&rcu_lock);
	list_for_each_safe(del_node, tmp, a_list)
        {
                entry = list_entry(del_node, struct k_list, test_list);
		list_del(del_node);
		kfree(entry);  
        }
	spin_unlock(&rcu_lock);

}

static int __init create_list_init(void) 
{

	
	thread_t = kmalloc(sizeof(*thread_t) * NUM_OF_THREADS, GFP_KERNEL);

	if (!thread_t) { 

		printk(KERN_ERR "Allocation error (thread_t)");	
		goto errn;
	}

	flags_stat.first_thread_active = 1;
	thread_t[0] = kthread_run(&first_thread_func, NULL, "mykthread_1");
		
	flags_stat.second_thread_active = 1;
	thread_t[1] = kthread_run(&second_thread_func, NULL, "mykthread_2");

	INIT_DELAYED_WORK(&work, work_handler);
	schedule_delayed_work(&work, msecs_to_jiffies(100));

	timer_setup(&timer, &timer_handler, 0);
	mod_timer(&timer, jiffies + msecs_to_jiffies(150));
	

	return 0;

	errn:

   		return -ENOMEM;

}
	
static void __exit create_list_exit(void) 
{
	cancel_delayed_work(&work);
	del_timer_sync(&timer);

	if (flags_stat.first_thread_active)
		kthread_stop(thread_t[0]);
	if (flags_stat.second_thread_active)
		kthread_stop(thread_t[1]);

	delete_list(&head_first_list);
	delete_list(&head_second_list);
	
	kfree(thread_t);
	printk(KERN_ALERT "Module exit\n");	
}

module_init(create_list_init);
module_exit(create_list_exit); 
