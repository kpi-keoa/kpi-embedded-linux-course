/* Based on https://bit.ly/2kLBtD9 made by thodnev.
 * This code was implemented by:  
 *	-Olexander Ostrianko
 *	-Maxim Salim
 *	-Dana Dovzhenko
 * Worque was added by Dana Dovzhenko, timer - by Maxim Salim. Synchronization
 * and all other work was done by Olexander Ostrianko. 
 * p.s. Everybody put a piece of themselves in this code <3.
 */

#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>	// used by kmod_init, kmod_exit macros
#include <linux/kthread.h>	// used by kernel threads
#include <linux/list.h>	// used by lists
#include <linux/slab.h>	// used for memory allocating
#include <linux/timer.h>	// used by kernel timer
#include <linux/workqueue.h>	// used by workque

MODULE_DESCRIPTION("Basic module demo: timer, worque, list, kthread");
MODULE_AUTHOR("AlexOstrianko; maksimo0; DovzhenkoD");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");// this affects the kernel behavior

static void tmr_handler(struct timer_list *data);
DEFINE_TIMER(mytimer, tmr_handler);
static void work_handler(struct work_struct *data);
DECLARE_DELAYED_WORK(mywork, work_handler);

struct struct_for_res {
	struct list_head list;
	long int num;
};

static struct struct_for_res frst_list;
static struct struct_for_res scnd_list;
struct task_struct **kth_ptr = NULL;

bool frst_kth_flag;
bool scnd_kth_flag;
bool tmr_flag;
bool wrk_flag;

static void tmr_handler(struct timer_list *data)
{
	long j = jiffies;
	struct struct_for_res *tmr_ptr = NULL;
	if (j%11 == 0) {
		printk(KERN_INFO "Stop timer jiffies value is: %li\n", j);
		frst_kth_flag = false;
	} else {
		tmr_ptr = kmalloc(sizeof *tmr_ptr, GFP_ATOMIC);
		if (tmr_ptr) {
			tmr_ptr->num = j;
			list_add(&tmr_ptr->list, &frst_list.list);
		} /*else {
			printk(KERN_ERR "kmalloc didn`t allocate memory for timer!\n");
			frst_kth_flag = false;
		} */	
		if (tmr_flag) {
			mod_timer(&mytimer, jiffies + 17);
		}
	}
}

static void work_handler(struct work_struct *data)
{
	long int j = jiffies;
	struct struct_for_res *wrk_ptr = NULL;	
	if (j%11 == 0) {
		printk(KERN_INFO "Stop work jiffies value is: %li\n", j);
		scnd_kth_flag = false;
	} else {
		wrk_ptr = kmalloc(sizeof *wrk_ptr, GFP_KERNEL);
		if (wrk_ptr) {
			wrk_ptr->num = j;
			list_add(&wrk_ptr->list, &scnd_list.list);
		} /*else {
			printk(KERN_ERR "kmalloc didn`t allocate memory for work!\n");
			scnd_kth_flag = false;
		}*/
		if (wrk_flag) {
			schedule_delayed_work(&mywork, 17);
		}
	}	
}

int frst_kth_func(void *data)
{
	while (frst_kth_flag) {
		schedule();
	}
	struct struct_for_res *frst_kth_ptr = NULL;
	list_for_each_entry(frst_kth_ptr, &(frst_list.list), list) {
		printk(KERN_NOTICE "Timer list element #%li!\n",
			frst_kth_ptr->num);
	}
	do_exit(1);
}

int scnd_kth_func(void *data)
{
	while (scnd_kth_flag) {
		schedule();
	}
	struct struct_for_res *scnd_kth_ptr = NULL;
	list_for_each_entry(scnd_kth_ptr, &(scnd_list.list), list) {
		printk(KERN_NOTICE "Work list element #%li!\n",
			scnd_kth_ptr->num);
	}
	do_exit(1);
}

static int __init kmod_init(void)
{
	INIT_LIST_HEAD(&frst_list.list);
	INIT_LIST_HEAD(&scnd_list.list);
	kth_ptr = kmalloc(sizeof *kth_ptr * 2, GFP_KERNEL);
	if (!kth_ptr) 
		goto kth_Error;
	frst_kth_flag = true;
	kth_ptr[0] = kthread_create(&frst_kth_func, (void *)0, "kthread_0");
	if(kth_ptr[0])
		wake_up_process(kth_ptr[0]);
	scnd_kth_flag = true;
	kth_ptr[1] = kthread_create(&scnd_kth_func, (void *)1, "kthread_1");
	if(kth_ptr[1])
		wake_up_process(kth_ptr[1]);
	INIT_LIST_HEAD(&frst_list.list);
	INIT_LIST_HEAD(&scnd_list.list);
	tmr_flag = true;
	mod_timer(&mytimer, jiffies + msecs_to_jiffies(1000));
	wrk_flag = true;
	schedule_delayed_work(&mywork, msecs_to_jiffies(1100));
	printk(KERN_INFO "Module is setup!");
	return 0;
	
	kth_Error:
		//printk(KERN_ERR "kmalloc didn`t allocate memory for threads!\n");
		return 0;
}

static void __exit kmod_exit(void)
{
	if (tmr_flag) {
		tmr_flag = false;
		del_timer_sync(&mytimer);
	}
	if (wrk_flag) {
		wrk_flag = false;
		while (flush_delayed_work(&mywork));
	}
	if (frst_kth_flag) {
		frst_kth_flag = false;
		kthread_stop(kth_ptr[0]);
	} 	
	if (scnd_kth_flag) {
		scnd_kth_flag = false;
		kthread_stop(kth_ptr[1]);
	}
	struct struct_for_res *struct_ptr;
	struct list_head *head_ptr;
	struct list_head *tmp;
	if (frst_list.list.next) {
		list_for_each_safe(head_ptr, tmp, &(frst_list.list)) {
			struct_ptr = list_entry(head_ptr, struct struct_for_res,
				 list);
			list_del(head_ptr);
			kfree(struct_ptr);		
		}
	}
	if (scnd_list.list.next) {
		list_for_each_safe(head_ptr, tmp, &(scnd_list.list)) {
			struct_ptr = list_entry(head_ptr, struct struct_for_res,
				 list);
			list_del(head_ptr);
			kfree(struct_ptr);		
		}
	}
	if (kth_ptr)
		kfree(kth_ptr);
	printk(KERN_NOTICE "Module unloaded!\n" ); 
}

module_init(kmod_init);
module_exit(kmod_exit);

