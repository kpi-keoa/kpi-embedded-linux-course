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

MODULE_DESCRIPTION("kernel threads");
MODULE_AUTHOR("Cartman"); 
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

unsigned long global_var = 0;

static unsigned long thread_num = 0;
module_param(thread_num, ulong, 0);
MODULE_PARM_DESC(thread_num, "thread count param");

static  unsigned short thread_delay = 0;
module_param(thread_delay, ushort, 0);
MODULE_PARM_DESC(thread_delay, "Delay (in ms) between iterations");

static unsigned int thread_inccnt = 0;
module_param(thread_inccnt, uint, 0);
MODULE_PARM_DESC(thread_inccnt, "Increment count");

static struct task_struct **threads;
static LIST_HEAD(tlist);

typedef struct __timeval {
	struct list_head ptr;
	typeof(jiffies) time;
} timeval;

static int thread_func(void *data)
{
	int i;
	typeof(jiffies) start_jiff;
	
	printk(KERN_INFO "process [%d] is running\n", current->pid);
	for(i = 0; i < thread_inccnt; ++i) {
		timeval *t = kzalloc(sizeof(*t), GFP_KERNEL);
		start_jiff = jiffies;
		
		++(*(volatile unsigned long *)data);
		
		schedule_timeout_uninterruptible(msecs_to_jiffies(thread_delay));
		
		t->time = jiffies_to_msecs(jiffies - start_jiff);
		list_add_tail(&(t->ptr), &tlist);
		
		if (kthread_should_stop())
			return 0;
	}
	printk(KERN_INFO "process [%d] is completed\n", current->pid);
	return 0;
}

static int __init lab4_init(void)
{
	int i;
	
	printk(KERN_INFO "%s: Initial Jiffies is %lu\n", 
		module_name(THIS_MODULE), jiffies);
	
	if (thread_num <= 0) {
		printk(KERN_ERR "Wrong thread_num\n");
		goto epic_fail;
	}
	
	if (thread_inccnt < 0) {
		printk(KERN_ERR "Wrong cnt\n");
		goto epic_fail;
	}
	
	if (thread_delay < 0) {
		printk(KERN_ERR "Wrong delay\n");
		goto epic_fail;
	}
	
	threads = kzalloc(thread_num * sizeof(*threads), GFP_KERNEL);
	if(threads == NULL) {
		printk(KERN_ERR "Mem alloc error\n");
		goto epic_fail;
	}
	for(i = 0; i < thread_num; i++) {
		threads[i] = kthread_run(thread_func, &global_var, "thread-%d", i);
		get_task_struct(threads[i]);
	}
	return 0;

epic_fail:
	printk(KERN_ERR "Deleted\n");
	return -1;
}

static void __exit lab4_exit(void)
{
	int i;
	struct list_head *pos = NULL;
	struct list_head *tmp;
	
	for (i = 0; i < thread_num; i++) {
		kthread_stop(threads[i]);
		put_task_struct(threads[i]);
	}
	kfree(threads);
	
	list_for_each_safe(pos, tmp, &tlist)
	{
		timeval *t = list_entry(pos, timeval, ptr);
		printk(KERN_INFO "Delta time = %ldms", t->time);
		list_del(pos);
		kfree(t);
	}
	
	printk(KERN_INFO "global var: %ld", global_var);
	printk(KERN_INFO "%s: exit\n", module_name(THIS_MODULE));
}
 
module_init(lab4_init);
module_exit(lab4_exit);

