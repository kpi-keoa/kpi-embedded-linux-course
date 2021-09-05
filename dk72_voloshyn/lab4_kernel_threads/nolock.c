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

MODULE_DESCRIPTION("Mutex synch");
MODULE_AUTHOR("Sad"); 
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

static unsigned int thread_num = 0;
module_param(thread_num, ulong, 0);
MODULE_PARM_DESC(thread_num, "Number of thread");

static  unsigned int thread_delay = 0;
module_param(thread_delay, ushort, 0);
MODULE_PARM_DESC(thread_delay, "Delay from 1 thread iteration to 2 thread iteration");

static unsigned int thread_inccnt = 0;
module_param(thread_inccnt, uint, 0);
MODULE_PARM_DESC(thread_inccnt, "Count of increment");

static struct task_struct **threads;
static LIST_HEAD(sadlist);
static volatile unsigned long sad_var = 0;

static DEFINE_MUTEX(mysad_mutex);

struct sadhead {  
	struct list_head sad_head;
	typeof(sad_var) sad_time;
};

static int sad_func(void *data)
{
	int i;
	typeof(jiffies) start_jiff;
	
	for(i = 0; i < thread_inccnt; ++i) {
		struct sadhead *sad = kzalloc(sizeof(*sad), GFP_KERNEL);
		start_jiff = jiffies;
		
		(*(volatile unsigned long *)data)++;

		schedule_timeout_uninterruptible(msecs_to_jiffies(thread_delay));
		
		sad->sad_time = jiffies_to_msecs(jiffies - start_jiff);
		list_add_tail(&(sad->sad_head), &sadlist);
		
		if (kthread_should_stop())
			return 0;
	}	

	return 0;
}

static int __init sadmod_init(void)
{
	int status = 0;	
	
	if (thread_num <= 0) {
		printk(KERN_ERR "Wrong thread_num\n");
		status = -EINVAL;
		goto error;
	}
	
	if (thread_inccnt < 0) {
		printk(KERN_ERR "Wrong cnt\n");
		status = -EINVAL;
		goto error;
	}
	
	if (thread_delay < 0) {
		printk(KERN_ERR "Wrong delay\n");
		status = -EINVAL;
		goto error;
	}
	
	threads = kzalloc(thread_num * sizeof(*threads), GFP_KERNEL);
	if(threads == NULL) {
		printk(KERN_ERR "Mem alloc error\n");
		status = -ENOMEM;
		goto error;
	}

	int s;

	for(s = 0; s < thread_num; s++) {
		threads[i] = kthread_run(sad_func, &sad_var, "thread-%d", s);
		get_task_struct(threads[s]);
	}
	return 0;

error:
	return status;
}

static void __exit sadmod_exit(void)
{
	int i;
	struct list_head *ptr = NULL;
	struct list_head *tmp;
	
	for (i = 0; i < thread_num; i++) {
		kthread_stop(threads[i]);
		put_task_struct(threads[i]);
	}
	kfree(threads);
	
	list_for_each_safe(ptr, tmp, &sadlist)
	{
		struct sadhead *sad = list_entry(ptr, struct sadhead, sad_head);
		printk(KERN_INFO "Sad delay time is %ld ms", sad->sad_time);
		printk(KERN_INFO "Sad variable after increment = %ld", sad_var);
		list_del(ptr);
		kfree(sad);
	}
	
	printk(KERN_INFO "Sad variable is - %ld now", sad_var);
}
 
module_init(sadmod_init);
module_exit(sadmod_exit);

