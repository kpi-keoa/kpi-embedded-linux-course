#include <linux/module.h> // required by all modules
#include <linux/kernel.h> // required for sysinfo
#include <linux/init.h> // used by module_init, module_exit macros
#include <linux/jiffies.h> // where jiffies and its helpers reside
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/list.h>

MODULE_DESCRIPTION("threads");
MODULE_AUTHOR("AlexShlikhta");
MODULE_VERSION("1.0");
MODULE_LICENSE("Dual MIT/GPL"); // this affects the kernel behavior

static int thread_num = 0;
module_param(thread_num, int, 0);
MODULE_PARM_DESC(thread_num, "number of threads");

static int thread_delay = 0;
module_param(thread_delay, int, 0);
MODULE_PARM_DESC(thread_delay, "delay between threads(in miliseconds)");

static int thread_inccnt = 0;
module_param(thread_inccnt, int, 0);
MODULE_PARM_DESC(thread_inccnt, "number of addings");

static struct task_struct **thread;
static volatile unsigned long glob_var = 0;
static LIST_HEAD(list);

struct _tmp_head {
	struct list_head head;
	typeof(glob_var) gfl;
};

static int foo(void *var)
{
	int i = 0;

	struct _tmp_head *tmp = kzalloc(sizeof(*tmp), GFP_KERNEL);

	for(i = 0;i < thread_inccnt;i++) {

		(*(typeof(glob_var)*)var)++;

		msleep(thread_delay);

		if(kthread_should_stop())
			break;
	}
	
	tmp->gfl = (*(typeof(glob_var)*)var);

	list_add_tail(&(tmp->head), &list);

	printk(KERN_INFO "thread is finished)\n");

	return 0;
}

static int __init mod_init(void)
{
	int status = 0;
	int i = 0;

	if(thread_num < 0) {
		printk(KERN_ERR "Invalid <thread_num> <= 0 :(\n");
		status = -EINVAL;
		goto final;
	}

	if(thread_delay < 0) {
		printk(KERN_ERR "Invalid <thread_delay> <= 0 :(\n");
		status = -EINVAL;
		goto final;
	}

	if(thread_inccnt < 0) {
		printk(KERN_ERR "Invalid <thread_inccnt> <= 0 :(\n");
		status = -EINVAL;
		goto final;
	}

	thread = kzalloc(thread_num * sizeof(*thread), GFP_KERNEL);
	if (thread == NULL) {
		printk(KERN_ERR "Couldn't allocate a memory:(\n");
		status = -ENOMEM;
		goto final;
	}


	for(i = 0; i < thread_num; i++) {
		thread[i] = kthread_run(foo, (void*)(&glob_var), "thread_%d", i);
		get_task_struct(thread[i]);
	}

final:
	return status;

}

static void __exit mod_exit(void)
{
	int i = 0;
	struct list_head *pos = NULL;
	struct list_head *next;

	for(i = 0; i < thread_num; i++){
		kthread_stop(thread[i]);
		put_task_struct(thread[i]);
	}
	kfree(thread);

	list_for_each_safe(pos, next, &list) {
		struct _tmp_head *tmp = list_entry(pos, struct _tmp_head, head);
		printk(KERN_INFO "glob_var per thread = %ld\n", tmp->gfl);
		list_del(pos);
		kfree(tmp);
	}

	printk(KERN_INFO "Final glob_var is %ld\n", glob_var);
	printk(KERN_INFO "Goodbye:)\n");
}

module_init(mod_init);
module_exit(mod_exit);
