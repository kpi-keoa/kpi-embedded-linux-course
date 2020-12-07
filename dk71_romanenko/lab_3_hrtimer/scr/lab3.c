#include <linux/module.h>	// required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/mm.h>

MODULE_DESCRIPTION("high-resolution timer module");
MODULE_AUTHOR("Cartman"); 
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");	//it affects the kernel behavior

static int cnt = 0;
module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "number of addings");

static int delay = 0;
module_param(delay, int, 0);
MODULE_PARM_DESC(cnt, "period in ms");

struct tmr_data {
	typeof(cnt) cnt;
	typeof(delay) delay;
	ktime_t ktime;
	struct hrtimer tim;
};

static struct tmr_data t_data;

static typeof(jiffies) *array = NULL;
static int cnt_t = 0;
static int i = 0;

void tasklet_best(unsigned long arg)
{
	printk(KERN_INFO "Tasklet jiffies is %lu\n", jiffies);
}

DECLARE_TASKLET(tasklet, tasklet_best, 0);

enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
	struct tmr_data *mytimer;
	mytimer = container_of(timer, struct tmr_data, tim);
	
	if(cnt_t < mytimer->cnt) {
		array[cnt_t++ % mytimer->cnt] = jiffies;
		return HRTIMER_RESTART;
	}
	return HRTIMER_NORESTART;
}

static int __init lab3_init(void)
{
	t_data.cnt = cnt;
	t_data.delay = delay;
	
	printk(KERN_INFO "Initial Jiffies is %lu\n", jiffies);
	tasklet_schedule(&tasklet);

	if ((array = kzalloc(t_data.cnt * sizeof(*array), GFP_KERNEL)) == NULL) {
		printk(KERN_ERR "Allocate error\n");
		goto epic_fail;
	}

	if (t_data.cnt <= 0) {
		printk(KERN_ERR "Wrong cnt\n");
		goto epic_fail;
	}

	if (t_data.delay < 0) {
		printk(KERN_ERR "Wrong delay\n");
		goto epic_fail;
	}
	
	t_data.ktime = ktime_set(0, t_data.delay * 1000000);
	
	/**
	* CLOCK_MONOTONIC use jiffies 
	* HRTIMER_MODE_REL - should be interpreted relative to the current time  
	*/  В
	hrtimer_init(&t_data.tim, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	t_data.tim.function = &timer_callback;
	hrtimer_start(&t_data.tim, t_data.ktime, HRTIMER_MODE_REL);
	return 0;

epic_fail:
	printk(KERN_ERR "Deleted\n");
	return -1;
}

static void __exit lab3_exit(void)
{
	printk(KERN_INFO "Exit jiffies is %lu\n", jiffies);
	
	hrtimer_cancel(&t_data.tim);
	
	if(cnt_t < t_data.cnt)
	printk(KERN_WARNING "Not passed\n");
	
	for (i = 0; i < cnt_t; i++) 
		printk(KERN_INFO "Array[%d] = %lu\n", i, array[i]);
		
	kfree((const void *)array);
	printk(KERN_INFO "Exit\n");
}
 
module_init(lab3_init);
module_exit(lab3_exit);

