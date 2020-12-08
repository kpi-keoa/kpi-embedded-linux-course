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
MODULE_AUTHOR("morel");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

static unsigned long th_val = 0;
module_param(th_val, ulong, 0);
MODULE_PARM_DESC(th_val, "magic");

static  unsigned short wrk_val = 0;
module_param(wrk_val, ushort, 0);
MODULE_PARM_DESC(wrk_val, "magic 2");

static unsigned int jffdelay = 0;
module_param(jffdelay, uint, 0);
MODULE_PARM_DESC(jffdelay, "delay time in jiffies");

static struct task_struct *thread_wrk;
static struct task_struct *thread_tim;

static DECLARE_COMPLETION(wrk_thread_done);
static DECLARE_COMPLETION(tim_thread_done);

static LIST_HEAD(tlist);
static LIST_HEAD(wlist);

struct tmr_data {
	typeof(jffdelay) jffdelay;
	typeof(th_val) th_val;
	struct timer_list tim;
};

struct wrk_data {
	typeof(jffdelay) jffdelay;
	typeof(th_val) wrk_val;
	struct delayed_work wrk;
};

struct time_val {
	struct list_head ptr;
	typeof(jiffies) time;
};

static struct tmr_data t_data;
static struct wrk_data w_data;

static bool t_stop = false;
static bool w_stop = false;

static int thread_func_wrk(void *data)
{
	struct list_head *pos = NULL;
	struct list_head *tmp;

	printk(KERN_INFO "process [%d] is running\n", current->pid);
	while(1) {
	if (kthread_should_stop() || w_stop) {
			printk(KERN_INFO "thread_func_wrk finished\n");
			list_for_each_safe(pos, tmp, &wlist)
			{
				struct time_val *t = list_entry(pos, struct time_val, ptr);
				printk(KERN_INFO "wrk time = %ld\n", t->time);
				list_del(pos);
				kfree(t);
			}
			return 0;
		}
	}
	return 0;
}
static int thread_func_tim(void *data)
{
	struct list_head *pos = NULL;
	struct list_head *tmp;

	printk(KERN_INFO "process [%d] is running\n", current->pid);
	while(1) {
	if (kthread_should_stop() || t_stop) {
			printk(KERN_INFO "thread_func_tim	 finished\n");
			list_for_each_safe(pos, tmp, &tlist)
			{
				struct time_val *t = list_entry(pos, struct time_val, ptr);
				printk(KERN_INFO "tim time = %ld\n", t->time);
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
	struct time_val *tmp = kzalloc(sizeof(*tmp), GFP_ATOMIC);    //atomic
	tmp->time = jiffies;

	struct wrk_data *data;
	data = container_of((struct delayed_work *)w, struct wrk_data, wrk);

	if (jiffies % wrk_val == 0) {
		w_stop = 1;
	} else {
		list_add_tail(&(tmp->ptr), &wlist);
		schedule_delayed_work(&w_data.wrk, data->jffdelay);
	}
}

static void tim_func(struct timer_list *d)
{
	struct time_val *tmp = kzalloc(sizeof(*tmp), GFP_ATOMIC);    //atomic
	tmp->time = jiffies;

	struct tmr_data *data;
	data = container_of((struct timer_list *)d, struct tmr_data, tim);

	if (jiffies % th_val == 0) {
		t_stop = true;
	} else {
		list_add_tail(&(tmp->ptr), &tlist);
		mod_timer(&t_data.tim, jiffies + data->jffdelay);
	}
}

static int __init khmod_init(void)
{
	printk(KERN_INFO "%s: Module started",module_name(THIS_MODULE));

	if (th_val <= 0 || wrk_val <= 0 || jffdelay <= 0) {
		printk(KERN_ERR "Wrong initial value\n");
		goto fail;
	}

	thread_wrk = kzalloc(sizeof(thread_wrk), GFP_KERNEL);
	thread_tim = kzalloc(sizeof(thread_tim), GFP_KERNEL);

	if(thread_tim == NULL || thread_wrk == NULL) {
		printk(KERN_ERR "threads mem alloc error\n");
		goto fail;
	}

	thread_wrk = kthread_run(thread_func_wrk, NULL, "thread-wrk");
	if (IS_ERR(thread_wrk)) {
		if(!PTR_ERR(thread_wrk)) {
			printk(KERN_ERR "thread start error\n");
			goto fail;
		}
	}
	get_task_struct(thread_wrk);

	thread_tim = kthread_run(thread_func_tim, NULL, "thread-tim");
	if (IS_ERR(thread_tim)) {
		if(!PTR_ERR(thread_tim)) {
			printk(KERN_ERR "thread start error\n");
			goto fail;
		}
	}
	get_task_struct(thread_tim);

	t_data.jffdelay = jffdelay;

	t_data.tim.expires = jiffies + jffdelay;
	timer_setup(&t_data.tim, tim_func, 0);
	mod_timer(&t_data.tim, jiffies);

	INIT_DELAYED_WORK(&w_data.wrk, &work_func);
	schedule_delayed_work(&w_data.wrk, 0);

	return 0;
fail:
	printk(KERN_ERR "Terminated\n");
	return -1;
}

static void __exit khmod_exit(void)
{
	printk(KERN_INFO "%s: exit\n", module_name(THIS_MODULE));

	del_timer(&t_data.tim);
	cancel_delayed_work(&w_data.wrk);

	if (!t_stop) {
		kthread_stop(thread_tim);
		put_task_struct(thread_tim);
	}
	kfree(thread_tim);

	if (!w_stop) {
		kthread_stop(thread_wrk);
		put_task_struct(thread_wrk);
	}
	kfree(thread_wrk);
}

module_init(khmod_init);
module_exit(khmod_exit);
