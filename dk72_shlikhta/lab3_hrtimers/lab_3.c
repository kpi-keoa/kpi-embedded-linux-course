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

MODULE_DESCRIPTION("timers");
MODULE_AUTHOR("AlexShlikhta");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL"); // this affects the kernel behavior

static int cnt = 0;
module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "number of addings");

static int delay = 0;
module_param(delay, int, 0);
MODULE_PARM_DESC(cnt, "period of our timer(in seconds)");

static typeof(jiffies) *array;
struct hrtimer my_timer;
ktime_t period = 0;
static int tmp_cnt = 0;
static int i;

void tasklet_foo(unsigned long arg)
{
	printk(KERN_INFO "Tasklet jiffies is %lu\n", jiffies);
}

DECLARE_TASKLET(tasklet, tasklet_foo, 0);

enum hrtimer_restart array_foo(struct hrtimer *data)
{
	array[tmp_cnt] = jiffies;

	hrtimer_forward_now(&my_timer, period);

	return (tmp_cnt++ != cnt) ? HRTIMER_RESTART : HRTIMER_NORESTART;
}

static int __init mod_init(void)
{
	int status = 0;
	period = ktime_set(delay, 0);

	if (cnt <= 0) {
		printk(KERN_ERR "Invalid <cnt> <= 0 :(\n");
		status = -EINVAL;
		goto final;		
	}

	if (delay < 0) {
		printk(KERN_ERR "Invalid <delay> < 0 :(\n");
		status = -EINVAL;
		goto final;		
	}

	if ((array = kzalloc(cnt * sizeof(*array), GFP_KERNEL)) == NULL) {
		printk(KERN_ERR "Couldn't allocate a memory:(\n");
		status = -ENOMEM;
		goto final;
	}

	printk(KERN_INFO "%d sec is %lu jiffies\n",delay, msecs_to_jiffies(delay * 1000));
	printk(KERN_INFO "Init jiffies is %lu\n", jiffies);

	tasklet_schedule(&tasklet);
	
	hrtimer_init(&my_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	my_timer.function = &array_foo;
	hrtimer_start(&my_timer, period, HRTIMER_MODE_REL);

final:
	return status;
}

static void __exit mod_exit(void)
{
	hrtimer_cancel(&my_timer);
	tasklet_kill(&tasklet);

	printk(KERN_INFO "Exit jiffies is %lu\n", jiffies);

	if(tmp_cnt != cnt)
		printk(KERN_WARNING "Array isn't full\n");

	for (i = 0; i < tmp_cnt; ++i)
		printk(KERN_INFO "Array[%d] =\t\t%lu\n", i, array[i]);
	
	kfree((const void *)array);

	printk(KERN_INFO "Goodbye:)\n");
}

module_init(mod_init);
module_exit(mod_exit);
