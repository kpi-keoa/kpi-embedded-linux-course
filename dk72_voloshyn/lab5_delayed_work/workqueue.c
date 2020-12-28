#include <linux/module.h>	// required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/kthread.h>	//kernel threads
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/sched/task.h>

MODULE_DESCRIPTION("Module witch make magic using wrk");
MODULE_AUTHOR("Sad");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

static unsigned long th_val = 0;
module_param(th_val, uint, 0);
MODULE_PARM_DESC(th_val, "timer val");

static  unsigned short wrk_val = 0;
module_param(wrk_val, uint, 0);
MODULE_PARM_DESC(wrk_val, "work val");

static unsigned int jffdelay = 0;
module_param(jffdelay, uint, 0);
MODULE_PARM_DESC(jffdelay, "delay time for work and timer");

static LIST_HEAD(timerlist);
static LIST_HEAD(worklist);

struct time_val {
	struct list_head ptr;
	typeof(jiffies) time;
};

struct wrk_data {
	typeof(jffdelay) jffdelay;
	typeof(th_val) wrk_val;
	struct delayed_work wrk;
};

struct tmr_data {
	typeof(jffdelay) jffdelay;
	typeof(th_val) th_val;
	struct timer_list tim;
};

static struct task_struct *timer_thread;
static struct task_struct *work_thread;

static struct tmr_data t_data;
static struct wrk_data w_data;

static bool t_stop = 0;
static bool w_stop = 0;

static int thread_work(void *data)
{
	struct list_head *pos = NULL;
	struct list_head *tmp;

	while(1) {
	if (kthread_should_stop() || w_stop) {
			printk(KERN_INFO "Function of thread for work done\n");
			list_for_each_safe(pos, tmp, &worklist)
			{
				struct time_val *t = list_entry(pos, struct time_val, ptr);
				printk(KERN_INFO "work time = %ld\n", t->time);
				list_del(pos);
				kfree(t);
			}
			return 0;
		}
	}
	return 0;
}

static int thread_timer(void *data)
{
	struct list_head *pos = NULL;
	struct list_head *tmp;

	while(1) {
	if (kthread_should_stop() || t_stop) {
			printk(KERN_INFO "Function of thread for timer done\n");
			list_for_each_safe(pos, tmp, &timerlist)
			{
				struct time_val *t = list_entry(pos, struct time_val, ptr);
				printk(KERN_INFO "timer time = %ld\n", t->time);
				list_del(pos);
				kfree(t);
			}
			return 0;
		}
	}
	return 0;
}

static void work_func(struct work_struct *w)
{
	struct time_val *tmp = kzalloc(sizeof(*tmp), GFP_ATOMIC);
	tmp->time = jiffies;

	struct wrk_data *data;
	data = container_of((struct delayed_work *)w, struct wrk_data, wrk);

	if (jiffies % wrk_val == 0) {
		w_stop = 1;
	} else {
		list_add_tail(&(tmp->ptr), &worklist);
		schedule_delayed_work(&w_data.wrk, data->jffdelay);
	}
}

static void timer_func(struct timer_list *d)
{
	struct time_val *tmp = kzalloc(sizeof(*tmp), GFP_ATOMIC);
	tmp->time = jiffies;

	struct tmr_data *data;
	data = container_of((struct timer_list *)d, struct tmr_data, tim);

	if (jiffies % th_val == 0) {
		t_stop = 1;
	} else {
		list_add_tail(&(tmp->ptr), &tlist);
		mod_timer(&t_data.tim, jiffies + data->jffdelay);
	}
}

static int __init sadmod_init(void)
{
	int status = 0;

	if (th_val <= 0 || wrk_val <= 0 || jffdelay <= 0) {
		printk(KERN_ERR "One of values is wrong\n");
		status = -EINVAL;
		goto exit;
	}

	work_thread = kzalloc(sizeof(work_thread), GFP_KERNEL);
	timer_thread = kzalloc(sizeof(timer_thread), GFP_KERNEL);

	if(timer_thread == NULL || work_thread == NULL) {
		printk(KERN_ERR "threads mem alloc error\n");
		status = -ENOMEM;
		goto exit;
	}

	work_thread = kthread_run(thread_work, NULL, "thread-wrk");
	if (IS_ERR(work_thread)) {
		if(!PTR_ERR(work_thread)) {
			printk(KERN_ERR "thread start error\n");
			status = -EINVAL;
			goto exit;
		}
	}
	get_task_struct(work_thread);

	timer_thread = kthread_run(thread_timer, NULL, "thread-tim");
	if (IS_ERR(timer_thread)) {
		if(!PTR_ERR(timer_thread)) {
			printk(KERN_ERR "thread start error\n");
			status = -EINVAL;
			goto exit;
		}
	}
	get_task_struct(timer_thread);

	t_data.jffdelay = jffdelay;

	t_data.tim.expires = jiffies + jffdelay;
	timer_setup(&t_data.tim, timer_func, 0);
	mod_timer(&t_data.tim, jiffies);

	INIT_DELAYED_WORK(&w_data.wrk, &work_func);
	schedule_delayed_work(&w_data.wrk, 0);

	return 0;
exit:
	return status;
}

static void __exit sadmod_exit(void)
{
	del_timer(&t_data.tim);
	cancel_delayed_work(&w_data.wrk);

	if (!t_stop) {
		kthread_stop(timer_thread);
		put_task_struct(timer_thread);
	}
	kfree(timer_thread);

	if (!w_stop) {
		kthread_stop(work_thread);
		put_task_struct(work_thread);
	}
	kfree(work_thread);
}

module_init(sadmod_init);
module_exit(sadmod_exit);
