#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/slab.h>
#include <linux/interrupt.h>	// tasklets
#include <linux/hrtimer.h>	// high resolution timers
#include <linux/time.h>
#include <linux/types.h>


MODULE_DESCRIPTION("Timer wheel, tasklets");
MODULE_AUTHOR("Oldem");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static long count = 0;
static unsigned long delay = 0;
static unsigned long *jiff_arr;
static unsigned long  counts = 0;

/* counter: start timer n-times */
module_param(count, long, 0);
MODULE_PARM_DESC(count, "Amount of timer cycles");

/* delay parameter */
module_param(delay, ulong, 0);
MODULE_PARM_DESC(delay, "Delay in ms");

static void tasklet_handler(struct tasklet_struct *data);
DECLARE_TASKLET(my_tasklet, tasklet_handler);
static void tasklet_handler(struct tasklet_struct *data)
{
	pr_info( "%s: tasklet ", module_name(THIS_MODULE));
	pr_info( "%s: jiff = %llu\n", module_name(THIS_MODULE), get_jiffies_64());
}

/* timer declare & handle */
static struct hrtimer arr_timer;

static enum hrtimer_restart arr_timer_handler(struct hrtimer *timer)
{
	jiff_arr[counts] = get_jiffies_64();
	counts++;

	if (counts < count) {
		hrtimer_forward_now(&arr_timer, ms_to_ktime(delay));
		return HRTIMER_RESTART;
	} else {
		pr_info( "%s : count finished", module_name(THIS_MODULE));
		return HRTIMER_NORESTART;
	}
}

static int __init secondmod_module_init(void)
{
	pr_info("%s: jiff = %llu\n", module_name(THIS_MODULE), get_jiffies_64());

	/* validation */
	if (count == 0) {
		pr_err( "%s: count value not correct %u\n", module_name(THIS_MODULE), count);
		return 0;
	}

	if (delay == 0) {
		pr_err("%s: delay value not correct %u\n", module_name(THIS_MODULE), delay);
		return 0;
	}

	jiff_arr = kmalloc(sizeof(*jiff_arr) * count, GFP_KERNEL);
	if (NULL == jiff_arr) {
		pr_err( "%s : array couldn't be initialized \n", module_name(THIS_MODULE));
		return 0;
	}

	tasklet_schedule(&my_tasklet);

	hrtimer_init(&arr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	arr_timer.function = &arr_timer_handler;
	hrtimer_start(&arr_timer, ms_to_ktime(delay), HRTIMER_MODE_REL);

	return 0;
}

static void __exit secondmod_module_exit(void)
{
	uint32_t i = 0;

	pr_info( "%s : jiff = %llu\n", module_name(THIS_MODULE), get_jiffies_64());

	/* free the tasklet and timer */
	if (hrtimer_is_queued(&arr_timer)) {
		hrtimer_cancel(&arr_timer);
		pr_warn("%s: timer is dead\n", module_name(THIS_MODULE));
	}

	tasklet_kill(&my_tasklet);

	/* print out the array */
	for (i = 0; i < counts; i++)
		pr_info( "%s : arr[%u] = %u\n",module_name(THIS_MODULE), i, jiff_arr[i]);

	if (NULL != jiff_arr)
		kfree(jiff_arr);
}

module_init(secondmod_module_init);
module_exit(secondmod_module_exit);
