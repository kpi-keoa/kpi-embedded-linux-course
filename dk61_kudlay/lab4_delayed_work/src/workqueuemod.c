// made by MaksHolub
// modified by CYB3RSP1D3R

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

MODULE_DESCRIPTION("Workqueue example");
MODULE_AUTHOR("CYB3RSP1D3R");
MODULE_VERSION("1.0");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior


struct kern_list {
    	struct list_head lhead;
	struct rcu_head rcu;
	long long count_val;
};

struct flags {
	unsigned first_thread_active:1;
	unsigned second_thread_active:1;
};

static struct flags flags_stat = {0};

static DEFINE_SPINLOCK(rcu_lock);
struct timer_list timer;


LIST_HEAD(first_list);
LIST_HEAD(second_list); 

const int THREADS_NUM = 2;

struct delayed_work work;

struct task_struct **thread_t;

static void __exit clean_module(void);


static void print_list(struct list_head *plist)
{

        struct list_head *listptr;
        struct k_list *entry;
	printk(KERN_ALERT "Print jiffies for every save.\n");

	rcu_read_lock();

	list_for_each_entry_rcu(entry, plist, lhead) {
		printk(KERN_INFO "Jiffies =  %lld ", entry->count_val);
	}

	rcu_read_unlock();
}


static int first_thread_func(void *argument)
{
  
	while (flags_stat.first_thread_active) {

		 schedule();
   	}


	print_list(&first_list);

	return 0;
}


static int second_thread_func(void *argument)
{
	
	while (!kthread_should_stop()) {

		 schedule();
   	}

	print_list(&second_list);
	flags_stat.second_thread_active = 0;

	return 0;

}


void work_handler(struct work_struct *arg)
{
	
	if ((jiffies%11) == 0) {
		
		kthread_stop(thread_t[1]);
		flags_stat.second_thread_active = 0;		

	} else {

		struct k_list *data = kmalloc(sizeof(struct k_list), GFP_ATOMIC);
		if (data != NULL) {
			printk(KERN_ERR "Allocation error of node");
		} else {
			data->count_val = jiffies;
			spin_lock(&rcu_lock);
			list_add_rcu(&data->lhead, &second_list);
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

		struct k_list *data = kmalloc(sizeof(struct k_list), GFP_ATOMIC);
		if (data != NULL) {
			printk(KERN_ERR "Allocation error of node");
			clean_module();
		} else {
			data->count_val = jiffies;
			spin_lock(&rcu_lock);
			list_add_rcu(&data->lhead, &first_list);
			spin_unlock(&rcu_lock);
			mod_timer(&timer, jiffies + 17);
		}

	}
}


void delete_list(struct list_head *plist)
{

	if (NULL == plist)
		return;

	struct k_list *entry;
        struct list_head *cursor, *tmp;

	spin_lock(&rcu_lock);

	list_for_each_safe (cursor, tmp, plist) {
                entry = list_entry(cursor, struct k_list, lhead);
		list_del(cursor);
		kfree(entry);  
        }

	spin_unlock(&rcu_lock);

}

static int __init init_mod(void) 
{


	thread_t = kmalloc(sizeof(*thread_t) * THREADS_NUM, GFP_KERNEL);

	if (thread_t == NULL) { 

		printk(KERN_ERR "Allocation error (thread_t)");	
		clean_module();
		return -ENOMEM;
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

}
	
static void __exit clean_module(void) 
{
	cancel_delayed_work(&work);
	del_timer_sync(&timer);

	if (flags_stat.first_thread_active)
		kthread_stop(thread_t[0]);
	if (flags_stat.second_thread_active)
		kthread_stop(thread_t[1]);

	delete_list(&first_list);
	delete_list(&second_list);
	
	kfree(thread_t);
	printk(KERN_ALERT "Module successfuly uninstaled.\n");	
}

module_init(init_mod);
module_exit(clean_module); 

