#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>    // require to user terminal params
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/kthread.h>	//kernel threads
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/list.h>
#include <linux/sched.h> // execution scheduling
#include <linux/sched/task.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>


MODULE_DESCRIPTION("Workqueue for all");
MODULE_AUTHOR("Cartman");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior


static int th_val;
module_param(th_val, int, 0);
MODULE_PARM_DESC(th_val, "Timer value");

static int wrk_val;
module_param(wrk_val, int, 0);
MODULE_PARM_DESC(wrk_val, "Queue value");

static int jffdelay;
module_param(jffdelay, int, 0);
MODULE_PARM_DESC(jffdelay, "Amount of delay");


static bool while_flag = 1;
struct delayed_work *mine_work;
static LIST_HEAD(timer_list);
static LIST_HEAD(queue_list);

static struct task_struct **threads;
typedef struct __timeval {
	struct list_head ptr;
	unsigned long time;
} timeval;

static int timer_func(void *data)
{

	unsigned long engage_jiff;

	timeval *t = kzalloc(sizeof(*t), GFP_KERNEL);
	engage_jiff = jiffies;

	if((engage_jiff % th_val) == 0)
		while_flag = 0;

	schedule_timeout_uninterruptible(msecs_to_jiffies(jffdelay));
	t->time = jiffies_to_msecs(engage_jiff);
	list_add_tail(&(t->ptr), &timer_list);
	if (kthread_should_stop() || !while_flag)
		return 0;


        return 0;

}


static void queue_func(struct work_struct *w)
{

	unsigned long engage_jiff;

	timeval *t = kzalloc(sizeof(*t), GFP_KERNEL);
	engage_jiff = jiffies;

	if((engage_jiff % wrk_val) == 0)
		return;

	t->time = jiffies_to_msecs(engage_jiff);
	list_add_tail(&(t->ptr), &queue_list);

	schedule_delayed_work(mine_work, jffdelay);

}

static int queue_th_func(void *data)
{

	if (kthread_should_stop())
		return 0;

}


static int __init lab5_init(void)
{
	int status = 0;

	if (th_val <= 0) {
		pr_err("Invalid th_val <= 0 :(\n");
		status = -EINVAL;
		goto epic_fail;
	}

	if (wrk_val <= 0) {
		pr_err("Invalid wrk_val <= 0 :(\n");
		status = -EINVAL;
		goto epic_fail;
	}

	if (jffdelay <= 0) {
		
		status = -EINVAL;
		goto epic_fail;
	}

	if ((threads = kzalloc(2 * sizeof(*threads), GFP_KERNEL)) == NULL)
	{
		pr_err("Couldn't allocate a memory:(\n");
		status = -ENOMEM;
		goto epic_fail;
	}

	if ((mine_work = kzalloc(sizeof(*mine_work), GFP_KERNEL)) == NULL) 
	{
         	printk(KERN_ERR "Couldn't allocate a memory:(\n");
         	status = -ENOMEM;
         	goto epic_fail;
	}

	threads[0] = kthread_run(timer_func, NULL, "timer thread");
	get_task_struct(threads[0]);

	threads[1] = kthread_run(queue_th_func, NULL, "queue thread");
	get_task_struct(threads[1]);

	INIT_DELAYED_WORK(mine_work, &queue_func);
	schedule_delayed_work(mine_work, 0);


	return 0;


epic_fail:
	pr_err("Deleted\n");
	return status;
}


static void __exit lab5_exit(void)
{

	struct list_head *pos = NULL;
	struct list_head *tmp;

	pr_info("Exit jiffies is %lu\n", jiffies);
	cancel_delayed_work(mine_work);


	kthread_stop(threads[0]);
	put_task_struct(threads[0]);
	kthread_stop(threads[1]);
	put_task_struct(threads[1]);

	kfree(threads);

	list_for_each_safe(pos, tmp, &timer_list)
	{
		timeval *t = list_entry(pos, timeval, ptr);
		printk(KERN_INFO "Timer fail in jiff = %lu", t->time);
		list_del(pos);
		kfree(t);
	}

	list_for_each_safe(pos, tmp, &queue_list)
	{
		timeval *t = list_entry(pos, timeval, ptr);
		printk(KERN_INFO "Queue fail in jiff = %lu", t->time);
		list_del(pos);
		kfree(t);
	}


	pr_info("%s: exit\n", module_name(THIS_MODULE));

	pr_info("Hello Kernel!\n");
}


module_init(lab5_init);
module_exit(lab5_exit);
