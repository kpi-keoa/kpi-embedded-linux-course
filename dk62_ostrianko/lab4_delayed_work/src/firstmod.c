/* Based on https://bit.ly/2kLBtD9 made by thodnev.
 * This code was implemented by:  
 *	-Olexander Ostrianko
 *	-Maxim Salim
 *	-Dana Dovzhenko
 * Worque was added by Dana Dovzhenko, timer - by Maxim Salim. Synchronization
 * and all other work was done by Olexander Ostrianko. 
 */

#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/interrupt.h>
#include <linux/kthread.h>	// used by kernel threads
#include <linux/list.h>
#include <linux/slab.h>		// used for memory allocating
#include <asm/atomic.h>	
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/semaphore.h>
#include <linux/delay.h>



MODULE_DESCRIPTION("Basic module demo: work, timer, kthreads");
MODULE_AUTHOR("AlexOstrianko; maksimo0; DovzhenkoD");
MODULE_VERSION("0.4");
MODULE_LICENSE("Dual MIT/GPL");	

const short NUMBER_OF_KTHREADS = 2;

struct struct_for_saving_results {
	struct list_head list;
	long int num;
};

struct flags_str {
	bool first_thr_run;
	bool second_thr_run;
	bool timer_run;
	bool work_run;
};

struct flags_str flags;
struct delayed_work work;
struct timer_list timer;
struct struct_for_saving_results first_res_list;
struct struct_for_saving_results second_res_list;
struct task_struct **threads_pointer = NULL;

void work_func(struct work_struct *data)
{
	long int temp_j = jiffies;
	struct struct_for_saving_results *work_func_st_p = NULL;
	if ((temp_j%11) == 0) {
		printk(KERN_INFO "Final work val is: %li\n", temp_j);
		flags.second_thr_run = false;
	} else {
		/*if (!flags.work_run) 
			printk(KERN_INFO "I need to stop!\n");*/		//Used for quick shutdown test
		work_func_st_p = kmalloc(sizeof(*work_func_st_p), GFP_ATOMIC);
		if (work_func_st_p) {
			work_func_st_p->num = temp_j;
			list_add(&work_func_st_p->list, &second_res_list.list);
			//printk(KERN_INFO "w - %li", temp_j);			//Used for sampling test
		} if (flags.work_run) {
			schedule_delayed_work(&work, 17);
		}
	}
}


void timer_func(struct timer_list *data)
{
	long int temp_j = jiffies;
	struct struct_for_saving_results *timer_func_st_p = NULL;
	if ((temp_j%11) == 0) {
		printk(KERN_INFO "Final timer val is: %li\n", temp_j);
		flags.first_thr_run = false;
	} else {
		timer_func_st_p = kmalloc(sizeof(*timer_func_st_p), GFP_ATOMIC);
		if (timer_func_st_p) {
			timer_func_st_p->num = temp_j;
			list_add(&timer_func_st_p->list, &first_res_list.list);
			//printk(KERN_INFO "t - %li", temp_j);			//Used for sampling test
		} if (flags.timer_run) {
			mod_timer(&timer, jiffies + 17);
		}
	}
}

int first_th_func(void *data)
{	
	while (flags.first_thr_run) {
		schedule();
	}

	struct struct_for_saving_results *f_tmp_st_p = NULL;

	list_for_each_entry(f_tmp_st_p, &(first_res_list.list), list) {
		printk(KERN_NOTICE "Timer list element #%li!\n",
			f_tmp_st_p->num);
	}

	do_exit(0);
}

int second_th_func(void *data)
{
	while (flags.second_thr_run) {
		schedule();
	}

	struct struct_for_saving_results *s_tmp_st_p = NULL;
	
	list_for_each_entry(s_tmp_st_p, &(second_res_list.list), list) {
		printk(KERN_NOTICE "Work list element #%li!\n",
			s_tmp_st_p->num);
	}

	do_exit(0);
}

static int __init mod_init(void) 
{ 	
	threads_pointer = kmalloc(sizeof(*threads_pointer) * NUMBER_OF_KTHREADS,
		GFP_KERNEL);	
	if (!threads_pointer) 
		goto Error;

	threads_pointer[0] = kthread_run(&first_th_func, (void *)0, "thread_0");
	flags.first_thr_run = true;
	threads_pointer[1] = kthread_run(&second_th_func, (void *)1, "thread_1");
	flags.second_thr_run = true;
	
	INIT_LIST_HEAD(&first_res_list.list);
	INIT_LIST_HEAD(&second_res_list.list);

	INIT_DELAYED_WORK(&work, work_func);
	schedule_delayed_work(&work, msecs_to_jiffies(100));
	flags.work_run = true;

	timer_setup(&timer, &timer_func, 0);
	mod_timer(&timer, jiffies + msecs_to_jiffies(110));
	flags.timer_run = true;


	printk(KERN_INFO "Module is setup!\n");
	
Error:
	return 0; 
		
}

static void __exit mod_exit(void) 
{ 
	if (flags.work_run) {
		flags.work_run = false;
		while (flush_delayed_work(&work));
	}
	
	if (flags.timer_run) {
		flags.timer_run = false;
		del_timer_sync(&timer);
	}

	if (flags.first_thr_run) {
		flags.first_thr_run = false;
		kthread_stop(threads_pointer[0]);
	}
	if (flags.second_thr_run) {
		flags.second_thr_run = false;
		kthread_stop(threads_pointer[1]);
	}

	if (threads_pointer)
		kfree(threads_pointer);


	struct struct_for_saving_results *struct_pointer;
	struct list_head *head_pointer;
	struct list_head *temp;

	if (first_res_list.list.next) {
		list_for_each_safe(head_pointer, temp, &(first_res_list.list)) {
			struct_pointer = list_entry(head_pointer, 
				struct struct_for_saving_results, list);
			/*printk(KERN_NOTICE "Dfirst list element #%li!\n",	//Used for deleting test
				struct_pointer->num);*/
			list_del(head_pointer);
			kfree(struct_pointer);		
		}
	}

	if (second_res_list.list.next) {
		list_for_each_safe(head_pointer, temp, &(second_res_list.list)) {
			struct_pointer = list_entry(head_pointer, 
				struct struct_for_saving_results, list);
			/*printk(KERN_NOTICE "Dsecond list element #%li!\n",	//Used for deleting test
				struct_pointer->num);*/
			list_del(head_pointer);
			kfree(struct_pointer);		
		}
	}


	printk(KERN_NOTICE "Module unloaded!\n" ); 
}


module_init(mod_init);
module_exit(mod_exit);

