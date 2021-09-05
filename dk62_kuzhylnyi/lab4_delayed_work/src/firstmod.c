#include <linux/module.h>	
#include <linux/kernel.h>
#include <linux/init.h>		
#include <linux/threads.h>          
#include <linux/kthread.h>      
#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/workqueue.h>	
#include <linux/sched.h>	

MODULE_DESCRIPTION("Module for timer & workqueue researching");
MODULE_AUTHOR("jayawar");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior


/* thread pointers */
static struct task_struct *timer_kthread;
static struct task_struct *work_kthread;


/* kernel timer */
static struct timer_list base_timer;

/* kernel workqueue */
static void workqueue_fn(struct work_struct *work); 

struct workqueue_struct *own_workqueue;

/* declare and init workqueue */
static DECLARE_DELAYED_WORK(jiq_work, workqueue_fn);

/*Linked List Node*/
struct timer_set {
        struct list_head list;
	size_t bad_jiffies;
};


/*Declare and init the head node of the linked list*/
LIST_HEAD(head_list_tmr); 
LIST_HEAD(head_list_wq);

/* main functions for operate with linked list */
void print_list(struct list_head *klist);
void destroy_list(struct list_head *klist);


/* Timer Handler */
void timeout_handler(struct timer_list *t)
{
    	printk(KERN_INFO "Timer handler started \n");

	if( 0 == (jiffies % 11)) {
            	printk(KERN_INFO "Timer handler completed \n");
		return;
        }	
	
	/* Node jiffes container */
        static struct timer_set *jiff_node;
        if( NULL ==  (jiff_node = kmalloc(sizeof(*jiff_node), GFP_KERNEL)))
            goto tmr_alloc_error;
    
	jiff_node->bad_jiffies = jiffies;
	list_add_tail(&(jiff_node->list), &head_list_tmr);  

        /* handle after 17 jiffies again */
        mod_timer(&base_timer, jiffies + 17);
        return;
        
        tmr_alloc_error:
		printk(KERN_ERR "Cannot allocate memory for new timer node\n");
                return;
}


/* Body function of timer_kthread */
static int timer_fn(void *args)
{
        timer_setup(&base_timer, timeout_handler, 0);
        mod_timer(&base_timer, jiffies + 1);

	while (!kthread_should_stop()) {
          schedule();
        }
        
        del_timer_sync(&base_timer);
        
        do_exit(0);
        return (0);
}


static void workqueue_fn(struct work_struct *work) {
    
        printk(KERN_INFO "Work started \n");

    	if( 0 == (jiffies % 11)) {
            printk(KERN_INFO "Work completed \n");
            return;
        }

        /* Node jiffes container */
        static struct timer_set *jiff_node;
        if( NULL ==  (jiff_node = kmalloc(sizeof(*jiff_node), GFP_KERNEL)))
            goto wq_alloc_error;
    
	jiff_node->bad_jiffies = jiffies;
        list_add_tail(&(jiff_node->list), &head_list_wq);  

        /* restart work after 17 jiffies */        
        schedule_delayed_work(&jiq_work, 17);
        return;
        
        wq_alloc_error:
            printk(KERN_ERR "Cannot allocate memory for new wq node\n");
            return;
}


static int workqueue_thread_func(void *args) {
    
    own_workqueue = create_workqueue("own_wq");  
    printk(KERN_INFO "Work thread started \n");
    queue_delayed_work(own_workqueue, &jiq_work, 1);

    do_exit(0);
    return (0);
}


static int __init delayed_init(void)
{
	     	 
        printk(KERN_INFO "Module init");

   	int rv;
	timer_kthread = kthread_run(timer_fn, 0, "timer_thread");
	if (ERR_PTR(-ENOMEM) == timer_kthread) {
		rv = -ENOMEM;
		goto alloc_error;
	}

        work_kthread = kthread_run(workqueue_thread_func, 0, "work_thread");
	if (ERR_PTR(-ENOMEM) == work_kthread) {
		rv = -ENOMEM;
		goto alloc_error;
	}
	
        
	printk(KERN_INFO "timer_thread thread has been started");
	return (0);

	alloc_error:
		printk(KERN_ERR "Allocation error");
		return rv;
}


static void __exit delayed_exit(void)
{
    
        cancel_delayed_work_sync(&jiq_work);
        flush_workqueue(own_workqueue);
        destroy_workqueue(own_workqueue);

        kthread_stop(timer_kthread);
    
        printk(KERN_INFO "Bad jiffies timer produced \n");
        print_list(&head_list_tmr);  
        printk(KERN_INFO "Bad jiffies delayed workqueue produced \n");
        print_list(&head_list_wq);
         
        destroy_list(&head_list_tmr);
	destroy_list(&head_list_wq);
}

module_init(delayed_init);
module_exit(delayed_exit);


/* 
 * kernel linked list basic routines 
*/

void print_list(struct list_head *klist)
{
	if (NULL == klist)
		return;

        struct timer_set *temp;        
        list_for_each_entry(temp, klist, list) {
            printk(KERN_INFO "Node stored: [%ld]\n", temp->bad_jiffies);
        }
}

void destroy_list(struct list_head *klist)
{
	if (NULL == klist)
		return;
        
	struct timer_set *cursor, *tmp;        
        list_for_each_entry_safe(cursor, tmp, klist, list) {
            list_del(&cursor->list);
            kfree(cursor);
        }
        
}
