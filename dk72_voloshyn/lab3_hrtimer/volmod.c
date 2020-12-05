#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/htimer.h>
#include <linux/slab.h>
#include <linux/mm.h>

MODULE_DESCRIPTION("tasklets + timers");
MODULE_AUTHOR("SadAnton");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static int cnt_arr = 0;
static unsigned long *array;
static struct hrtimer timer;
static int cnt_print = 0;
static ktime_t ktime;

static int delay = 0;
module_param(delay, int, 0);
MODULE_PARM_DESC(delay, "timer's delay");

static int cnt = 0;
module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "Count of cycles, that the timer should be run");

void tasklet_print_jiff(unsigned long value)
{
	printk(KERN_INFO "Value of tasklet jiffies = %lu \n", jiffies);
}

DECLARE_TASKLET(tasklet, tasklet_print_jiff, 0);

enum hrtimer_restart timer_func(struct hrtimer *data)
{
	array[cnt_arr] = jiffies;
	hrtimer_forward_now(&timer, ktime);

	if(++cnt_arr < cnt)
		return HRTIMER_RESTART;
        return HRTIMER_NORESTART;
}

static int __init mod_init(void)
{
	printk(KERN_INFO "Value of jiffies = %lu \n", jiffies); // Print init's jiff
	tasklet_schedule(&tasklet);				// Print tasklet's jiff
	
	int status = 0;	
	
	if (cnt <= 0 || delay < 0) {
		printk(KERN_ERR "cnt <= 0 or delay < 0\n");
		status = -EINVAL;
		goto error;
   	}

	array = kzalloc(cnt * sizeof(*array), GFP_KERNEL);
	
	if (array == NULL) {
        	printk(KERN_ALERT "Fail in allocated");
        	status = -ENOMEM;
        	goto error;
    	}
	
	ktime = ktime_set(0, delay * 1000000);

    	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    	timer.function = &timer_funk;
    	hrtimer_start(&timer, ktime, HRTIMER_MODE_REL);
	
error:
	return status;
}

static void __exit mod_exit(void)
{
	tasklet_kill(&tasklet);
	hrtimer_cancel(&timer);

	printk(KERN_INFO "Exit jiffies is %lu\n", jiffies);

	if(cnt_arr != cnt)
		printk(KERN_WARNING "Array isn't full\n");

	for (cnt_print = 0; cnt_print < tmp_cnt; cnt_print++)
		printk(KERN_INFO "Array number - %d, jiff value - %lu", cnt_print, array[cnt_print]);
	
	kfree(array);

	printk(KERN_INFO "Vzhuh! \n");
}

module_init(mod_init);
module_exit(mod_exit);

