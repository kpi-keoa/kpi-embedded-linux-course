#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>   // where jiffies and its helpers reside
#include <linux/slab.h>
#include <linux/interrupt.h>    // require tasklets
#include <linux/hrtimer.h>

MODULE_DESCRIPTION("hrtimer");
MODULE_AUTHOR("artem-dk81");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static unsigned long *array;
static unsigned long ticks;

static long cnt = 5;
module_param(cnt, long, 0);
MODULE_PARM_DESC(cnt, "Amount of cycles that timer works");

static unsigned long delay = 100;
module_param(delay, ulong, 0);
MODULE_PARM_DESC(delay, "Delay of timer");


static void tasklet1_handler(struct tasklet_struct *data);
DECLARE_TASKLET(tasklet1, tasklet1_handler);

//static void timer1_handler(struct timer_list *list);
//DEFINE_TIMER(timer1, timer1_handler);

static struct hrtimer hrtimer_1;
static enum hrtimer_restart hrtimer_handler(struct hrtimer *timer);


static int __init module1_init(void)
{
	ticks = 0;

	// print Start jiffies
	pr_info("init module>> jiffies = %lu\n", jiffies);

	// enable schedule for tasklet1
	tasklet_schedule(&tasklet1);

	// check if cnt == 0
	if (0 == cnt) {
		pr_warn("init>> cnt = 0\n");
		return 0;
	}

	// allocate memory
	array = kmalloc(sizeof(cnt) * cnt, GFP_KERNEL);

	// check if mem.allocation failed
	if (NULL == array) {
		pr_err("init>> array = NULL\n");
		return 0;
	}

	// check if delay == 0
	if (0 == delay) {
		pr_warn("init>> delay = 0\n");
	}

	// change period of timer
	//mod_timer(&timer1, jiffies + delay);

	hrtimer_init(&hrtimer_1, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer_1.function = &hrtimer_handler;
	hrtimer_start(&hrtimer_1, ms_to_ktime(delay), HRTIMER_MODE_REL);
	return 0;
}

static void tasklet1_handler(struct tasklet_struct *data)
{
	pr_info("tasklet>> jiffies = %lu\n", jiffies);
}


static enum hrtimer_restart hrtimer_handler(struct hrtimer *timer)
{
	array[ticks] = jiffies;
	ticks++;

	if (ticks >= cnt) {
		return HRTIMER_NORESTART;
	}

	hrtimer_forward_now(&hrtimer_1, ms_to_ktime(delay));
	return HRTIMER_RESTART;
}

static void __exit module1_exit(void)
{
	long i;

	pr_info("exit module>> jiffies= %lu\n", jiffies);

	tasklet_kill(&tasklet1);

	if (hrtimer_is_queued(&hrtimer_1)) {
		pr_warn("exit >> timer is queued\n");
		pr_info("exit >> Killed timer_1\n");
		hrtimer_cancel(&hrtimer_1);
	}

	for (i = 0; i < ticks; i++) {
		pr_info("timer >> array[%lu] jiffies= %lu\n", i, array[i]);
	}

	if (NULL != array) {
		kfree(array);
	}
}

module_init(module1_init);
module_exit(module1_exit);