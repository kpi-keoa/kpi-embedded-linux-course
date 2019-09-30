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
#include <linux/init.h>	// used by kmod_init, kmod_exit macros
#include <linux/kthread.h>	// used by kernel threads
#include <linux/list.h>	// used by lists
#include <linux/slab.h>	// used for memory allocating
#include <linux/timer.h>	// used by kernel timer
#include <linux/workqueue.h>	// used by workque

MODULE_DESCRIPTION("Basic module demo: kthread, timer, workqueue, list");
MODULE_AUTHOR("AlexOstrianko; maksimo0; DovzhenkoD");
MODULE_VERSION("0.4");
MODULE_LICENSE("Dual MIT/GPL");	

const int NUMBER_OF_KTHREADS = 2;

struct struct_result_of_kth {
	struct list_head list;
	long int cnt;
};

struct flags_str {
	bool thr_run[2];
	bool timer_run;
	bool work_run;
};

struct flags_str flags;
struct delayed_work work;
struct timer_list timer;
struct struct_result_of_kth first_list;
struct struct_result_of_kth second_list;

struct task_struct **kthreads_ptr = NULL;

void work_func(struct work_struct *data)
{
	long int jif_work = jiffies;
	struct struct_result_of_kth *work_data;
	if ((jif_work % 11) == 0) {
		printk(KERN_INFO "\nElement/11 from work: %li, jiffies/11 = %li\n", 
			jif_work, jif_work/11);
	flags.thr_run[1] = false;
	} else {
		work_data = kmalloc(sizeof(*work_data), GFP_KERNEL);
		if (work_data) {
			work_data->cnt = jif_work;
			list_add(&work_data->list, &second_list.list);
		} else {
			printk(KERN_ERR "kmalloc didn`t allocate memory!\n");
			flags.thr_run[1] = false;			
		} if (flags.work_run) {
			schedule_delayed_work(&work, 17);
		}
	}

}


void timer_func(struct timer_list *data)
{

	long int jif_timer = jiffies;
	struct struct_result_of_kth *timer_data;
	if ((jif_timer % 11) == 0) {
		printk(KERN_INFO "\nElement/11 from timer: %li, jiffies/11 = %li\n", 
			jif_timer, jif_timer/11);
		flags.thr_run[0] = false;
	} else {
		timer_data = kmalloc(sizeof(*timer_data), GFP_ATOMIC);
	if (timer_data) {
			timer_data->cnt = jif_timer;
			list_add(&timer_data->list, &first_list.list);
		} else {
			printk(KERN_ERR "kmalloc didn`t allocate memory!\n");
			flags.thr_run[0] = false;			
		} if (flags.timer_run) {
			mod_timer(&timer, jiffies + 17);
		}
	}

}

int th_func(void *data)
{	
	if ((int *)data == 0) goto TIMER_KTH;
	if ((int *)data == 1) goto WORK_KTH;
	TIMER_KTH:
		while (flags.thr_run[0]) {
			schedule();
		}
		struct struct_result_of_kth *temp_t = NULL;
		list_for_each_entry(temp_t, &(first_list.list), list) {
		printk(KERN_NOTICE "\tThread - %i. Timer elements list #%li!\n", 
			(int *)data, temp_t->cnt);
		}
		do_exit(1);
	WORK_KTH:
		while (flags.thr_run[1]) {
			schedule();
		}
		struct struct_result_of_kth *temp_w = NULL;
		list_for_each_entry(temp_w, &(second_list.list), list) {
			printk(KERN_NOTICE "\tThread - %i. Work elements list #%li!\n", 
				(int *)data, temp_w->cnt);
		}
		do_exit(1);	
}

static int __init mod_init(void) 
{ 	
	kthreads_ptr = kmalloc(sizeof(*kthreads_ptr) * NUMBER_OF_KTHREADS,
		GFP_KERNEL);	

	if (!kthreads_ptr) 
		goto Error_kth;

	for (int i = 0; i < NUMBER_OF_KTHREADS; i ++) {
		kthreads_ptr[i] = kthread_run(&th_func, (void *)i, "thread_%i", i);
		flags.thr_run[i] = true;
	}

	INIT_LIST_HEAD(&first_list.list);
	INIT_LIST_HEAD(&second_list.list);

	INIT_DELAYED_WORK(&work, work_func);
	schedule_delayed_work(&work, msecs_to_jiffies(1000));
	flags.work_run = true;

	timer_setup(&timer, &timer_func, 0);
	mod_timer(&timer, jiffies + msecs_to_jiffies(1100));
	flags.timer_run = true;

	printk(KERN_INFO "Module is setup!");
	
	return 0; 

	Error_kth:
		printk(KERN_ERR "Kmalloc didn`t allocate memory!\n"); 
		kfree(kthreads_ptr);
		return 0; 
		
}

static void __exit mod_exit(void) 
{ 
	for (int i = 0; i < NUMBER_OF_KTHREADS; i++) {
		if (flags.thr_run[i]) {
		flags.thr_run[i] = false;
		kthread_stop(kthreads_ptr[i]);
		}
	}

	if (flags.work_run) {
		flags.work_run = false;
		while (flush_delayed_work(&work));
	}
	
	if (flags.timer_run) {
		flags.timer_run = false;
		del_timer_sync(&timer);
	}

	struct struct_result_of_kth *struct_ptr;
	struct list_head *head_ptr;
	struct list_head *temp;

	if (first_list.list.next) {
		list_for_each_safe(head_ptr, temp, &(first_list.list)) {
			struct_ptr = list_entry(head_ptr, 
				struct struct_result_of_kth, list);
			list_del(head_ptr);
			kfree(struct_ptr);		
		}
	}

	if (second_list.list.next) {
		list_for_each_safe(head_ptr, temp, &(second_list.list)) {
			struct_ptr = list_entry(head_ptr, 
				struct struct_result_of_kth, list);
			list_del(head_ptr);
			kfree(struct_ptr);		
		}
	}

	if (kthreads_ptr)
		kfree(kthreads_ptr);

	printk(KERN_NOTICE "Module unloaded!\n" ); 
}


module_init(mod_init);
module_exit(mod_exit);
