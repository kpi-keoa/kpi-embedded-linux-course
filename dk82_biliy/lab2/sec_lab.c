#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/time.h>

MODULE_DESCRIPTION(
    "Basic module which shows user_name parameter, and worktime");
MODULE_AUTHOR("vitaliy2034");
MODULE_VERSION("0.2");
MODULE_LICENSE("GPL");

// Parameters
static int32_t cnt;
static int32_t delay;

// Local wars
static uint64_t *jiff_arr;

module_param(cnt, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(cnt, "cnt is the number of cycles that the timer should run;");

module_param(delay, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(delay, "delay is the delay between two timer firings");

static void tasklet_handler(struct tasklet_struct *data);

DECLARE_TASKLET(task, tasklet_handler);

static void tasklet_handler(struct tasklet_struct *data)
{
	pr_info("Tasklet: jiffies = %llu\n", get_jiffies_64());
}

static enum hrtimer_restart timer_handler(struct hrtimer *timer)
{
	static int i = 0;

	jiff_arr[i] = jiffies;

	i += 1;

	if (i == cnt)
		return HRTIMER_NORESTART;

	hrtimer_forward_now(&cnt_timer, ms_to_ktime(delay));
	return HRTIMER_RESTART;
}

static int __init secondmod_init(void)
{
	pr_info("Init jiffies = %llu\n", get_jiffies_64());

	tasklet_schedule(&task);

	if (cnt <= 0) {
		printk(KERN_ERR "Error: invalid value of cnt!\n");
		return status;
	}
	if (delay <= 0) {
		printk(KERN_ERR "Error: invalid value of delay!\n");
		return status;
	}

	jiff_arr = kzalloc(cnt * sizeof(*jiff_arr), GFP_KERNEL);
	if (jiff_arr == NULL) {
		printk(KERN_ERR "Error: cannot allocate memory!\n");
		return ENOMEM;
	}

	hrtimer_init(&cnt_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	cnt_timer.function = &timer_handler;
	hrtimer_start(&cnt_timer, delay, HRTIMER_MODE_REL);
}

static void __exit secondmod_exit(void)
{
	uint64_t total_time_64 = get_jiffies_64() - init_jiffies_64;

	pr_info("Stop module\n Module works: %llu ms",
		jiffies64_to_msecs(total_time_64));

	if (jiff_arr == NULL) {
		tasklet_kill(&task);
		return;
	}
	if (hrtimer_is_queued(&cnt_timer)) {
		cnt = i;
		printk(KERN_INFO "array filling stoped!");
	}
	while (i > 0) {
		printk(KERN_INFO "jiff_arr[%lu] = %lu\n", cnt - i,
		       jiff_arr[cnt - i]);
		i--;
	}
	kfree(jiff_arr);
	hrtimer_cancel(&cnt_timer);
}

module_init(firstmod_init);
module_exit(firstmod_exit);
