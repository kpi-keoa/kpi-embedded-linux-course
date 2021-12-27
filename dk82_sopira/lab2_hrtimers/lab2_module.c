#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/slab.h>
#include <linux/interrupt.h>	// tasklets
#include <linux/hrtimer.h>	// high resolution timers
#include <linux/time.h>
#include <linux/types.h>	// bad, but i don't like remembering things

/* module defs */
#define MODULE_NAME "lab2_module"

MODULE_DESCRIPTION("Timer wheel, tasklets");
MODULE_AUTHOR("Ruslan Sopira");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

/* arg vars */
static uint32_t cnt = 4;
static uint32_t delay = 500;
/* inner vars */
static uint32_t *jiff_arr;
static uint32_t counts = 0;

/* counter: start timer n-times */
module_param(cnt, uint, 0);
MODULE_PARM_DESC(cnt, "Amount of timer cycles");

/* delay parameter */
module_param(delay, uint, 0);
MODULE_PARM_DESC(delay, "Timer delay in milliseconds");

/* misc functions */
static inline void print_jiffies(void)
{
	pr_info(MODULE_NAME ": jiff = %llu\n", get_jiffies_64());
}

/* tasklet declare */
static void tasklet_handler(struct tasklet_struct *data);
DECLARE_TASKLET(my_tasklet, tasklet_handler);

static void tasklet_handler(struct tasklet_struct *data)
{
	pr_info(MODULE_NAME ": tasklet strikes back!");
	print_jiffies();
}

/* timer declare & handle */
static struct hrtimer arr_timer;

static enum hrtimer_restart arr_timer_handler(struct hrtimer *timer)
{
	jiff_arr[counts] = get_jiffies_64();
	counts++;
	
	if (counts < cnt) {
		hrtimer_forward_now(&arr_timer, ms_to_ktime(delay));
		return HRTIMER_RESTART;
	} else {
		pr_info(MODULE_NAME ": finished counting successfuly!");
		return HRTIMER_NORESTART;
	}
}

/* INIT FUNC */
static int __init lab2_module_init(void)
{
	print_jiffies();
	
	/* validation */
	if (cnt == 0) {
		pr_err(MODULE_NAME ": cnt value not valid %u\n", cnt);
		return -2;
	}
	
	if (delay == 0) {
		pr_err(MODULE_NAME ": delay value not valid %u\n", delay);
		return -2;
	}

	jiff_arr = kmalloc(sizeof(*jiff_arr) * cnt, GFP_KERNEL);
	if (NULL == jiff_arr) {
		pr_err(MODULE_NAME ": could not initialize an array\n");
		return -2;
	}
	
	tasklet_schedule(&my_tasklet);
	
	/* initialize hrtimer */

	/* 	
	 * 	CLOCK_REALTIME
	 * resettable clock source that measures real time and
	 * can be manually changed by someone with required privileges
	 * i.e. can jump backward and forward
	 * 	CLOCK_MONOTONIC
	 * a linear clock source that is guaranteed to not go backwards:
	 * t2 is always bigger than t1
	 * i.e. nonsettable, and can not be affected by manual changes
	 * does not count time if the system is suspended
	 * 	CLOCK_BOOTTIME
	 * almost the same as CLOCK_MONOTONIC, but this one includes
	 * time when the system is suspended
	 */

	hrtimer_init(&arr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	arr_timer.function = &arr_timer_handler;
	hrtimer_start(&arr_timer, ms_to_ktime(delay), HRTIMER_MODE_REL);

	return 0;
}

/* EXIT FUNC */
static void __exit lab2_module_exit(void)
{
	uint32_t i = 0;
	
	print_jiffies();

	/* free the tasklet and timer */		
	if (hrtimer_is_queued(&arr_timer)) {
		hrtimer_cancel(&arr_timer);
		pr_warn(MODULE_NAME ": timer killed preemptively\n");
	}
	
	tasklet_kill(&my_tasklet);
	
	/* print out the array */
	for (i = 0; i < counts; i++)
		pr_info(MODULE_NAME ": arr[%u] = %u\n", i, jiff_arr[i]);

	if (NULL != jiff_arr)
		kfree(jiff_arr);
}

module_init(lab2_module_init);
module_exit(lab2_module_exit);
