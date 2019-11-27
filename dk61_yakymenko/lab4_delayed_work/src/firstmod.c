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
#include <linux/timer.h>
#include <linux/types.h>	
#include <linux/completion.h>
#include <linux/workqueue.h>

MODULE_DESCRIPTION("Kernel delay");
MODULE_AUTHOR("olegovichh22");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

int num_of_threads = 2;
static struct task_struct **thread = NULL;

void print_and_erase_list(struct list_head *print_list);

//declaration for complition
static DECLARE_COMPLETION(comp_timer);
static DECLARE_COMPLETION(comp_wq);

//declaration for list
static LIST_HEAD(list_timer_thread);
static LIST_HEAD(list_wq_thread);
struct long_node {
	struct list_head list;
	long data;
};

//declaration for workqueue
void wq_fun(struct work_struct *work);
DECLARE_DELAYED_WORK(wq_task, wq_fun);

static struct timer_list my_timer;

void timer_fun(struct timer_list *t)
{
    printk(KERN_INFO "TIMER %ld, %d\n", jiffies, jiffies%11);
    
    if((jiffies%11) == 0) {
    	printk(KERN_INFO "TIMER GOT TRUE JIFFIES %ld, %d\n", jiffies, jiffies%11);
    	complete(&comp_timer);
    }
    else {
       	struct long_node * node = kmalloc(sizeof(struct long_node), GFP_KERNEL);
       	if(NULL == node) {
       		printk(KERN_ERR "TIMER FUN: ERROR allocting memory\n");
       		complete(&comp_timer);
    		print_and_erase_list(&list_timer_thread);
       	}
       	else {
       		node->data = jiffies;
			list_add(&node->list, &list_timer_thread);
		
    		mod_timer(&my_timer, jiffies + 17);
       	}
    }
}

int thread_timer(void *arg)
{
	timer_setup(&my_timer, timer_fun, 0);
  	mod_timer(&my_timer, jiffies + msecs_to_jiffies(10000));
	
	wait_for_completion(&comp_timer);
	do_exit(0);
}

void wq_fun(struct work_struct *work)
{
    printk(KERN_INFO "WQ %ld, %d\n", jiffies, jiffies%11);
    
    if((jiffies%11) == 0) {
    	printk(KERN_INFO "WQ GOT TRUE JIFFIES %ld, %d\n", jiffies, jiffies%11);
    	complete(&comp_wq);
    }
    else {
       	struct long_node * node = kmalloc(sizeof(struct long_node), GFP_KERNEL);
       	if(NULL == node) {
       		printk(KERN_ERR "WQ FUN: ERROR allocting memory\n");
       		complete(&comp_wq);
   	 		print_and_erase_list(&list_wq_thread);
       	}
       	else {
       		node->data = jiffies;
			list_add(&node->list, &list_wq_thread);
		
    		schedule_delayed_work(&wq_task, 17);
       	}
    } 
    
}

int thread_wq(void *arg)
{
	schedule_delayed_work(&wq_task, msecs_to_jiffies(1000));
	wait_for_completion(&comp_wq);
	do_exit(0);
}

void print_and_erase_list(struct list_head *print_list)
{
	if(NULL == print_list) {
    	printk(KERN_ERR "Cant print NULL list\n");
      	return;
    }
	
	struct list_head *current_pos = NULL;
	struct list_head *tmp = NULL;
	struct long_node * current_node = NULL;
	list_for_each_safe(current_pos, tmp, print_list) {
         
         current_node = list_entry(current_pos, struct long_node, list);
         
         printk ("List element = %ld\n" , current_node->data);
         
         list_del(current_pos);
         kfree(current_node);
    }
}

static int __init kernel_thread_init(void)
{	
	thread = kmalloc(sizeof(struct task_struct **)*num_of_threads, GFP_KERNEL);
	if(NULL == thread) {
    	printk(KERN_ERR "MAIN THREAD: Cant allocate memory for threads\n");
      	return -1;
    }
    
	thread[0] = kthread_run(thread_timer, NULL, "thread_timer");
	
	if(ERR_PTR(-ENOMEM) == thread[0])
	printk(KERN_ERR "MAIN THREAD: thread_timer have not started\n");
	
	thread[1] = kthread_run(thread_wq, NULL, "thread_wq");
	
	if(ERR_PTR(-ENOMEM) == thread[0])
	printk(KERN_ERR "MAIN THREAD: thread_wq have not started\n");
	
	printk(KERN_INFO "Main thread!\n");
	
	return 0;
}



static void __exit kernel_thread_exit(void)
{
	printk(KERN_INFO "Closing the module\n");
	
	del_timer(&my_timer);
	cancel_delayed_work(&wq_task);
	
	complete(&comp_timer);
	complete(&comp_wq);
	
	printk(KERN_INFO "Timer list\n");
	print_and_erase_list(&list_timer_thread);
	
	printk(KERN_INFO "WQ list\n");
	print_and_erase_list(&list_wq_thread);
	
	kfree(thread);
	
	printk(KERN_INFO "See you later aligator!\n");
}
 
module_init(kernel_thread_init);
module_exit(kernel_thread_exit);
