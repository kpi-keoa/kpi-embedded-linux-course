#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>    // require to user terminal params
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/interrupt.h> // require tasklets
#include <linux/timer.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

MODULE_DESCRIPTION("Task for lab_3 ");
MODULE_AUTHOR("oleksandr lavrusenko");
MODULE_VERSION("3.0");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static int i;
static int cnt = 0;
static int tmp_cnt = 0;
static int delay;
static typeof(jiffies) *array;
static struct hrtimer hr_timer;
static ktime_t ktime;
static void my_tasklet_handler(unsigned long flag);

module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "Number of cycle");
module_param(delay, int, 0);
MODULE_PARM_DESC(delay, "Amount of delay");

DECLARE_TASKLET(my_tasklet, my_tasklet_handler, 0);

enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
        printk(KERN_INFO "Iteration %d\n", tmp_cnt);
        array[tmp_cnt++] = jiffies;
        hrtimer_forward_now(&hr_timer, ktime);
        if (tmp_cnt < cnt)
                return HRTIMER_RESTART;
        return HRTIMER_NORESTART;
}


static void my_tasklet_handler(unsigned long flag)
{
        printk(KERN_INFO "Tasklet jiffies %lu\n", jiffies);
}

static int __init lab3_init(void)
{
        int status = 0;

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

        printk(KERN_INFO "%d msec is %lu jiffies\n",delay, msecs_to_jiffies(delay));
        printk(KERN_INFO "Init jiffies is %lu\n", jiffies);

        tasklet_schedule(&my_tasklet);


        ktime = ktime_set(0, delay * 1000000);
        hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        hr_timer.function = &timer_callback;
        hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL);

final:
	return status;
}


static void __exit lab3_exit(void)
{
        tasklet_kill(&my_tasklet);
        hrtimer_cancel(&hr_timer);

        printk(KERN_INFO "Exit jiffies is %lu\n", jiffies);

        if(tmp_cnt != cnt)
                printk(KERN_WARNING "Array isn't full\n");

        for (i = 0; i < tmp_cnt; ++i)
                printk(KERN_INFO "Array[%d] = %lu\n", i, array[i]);

        kfree((const void *)array);

        printk(KERN_INFO "Ave Kernel!\n");
}


module_init(lab3_init);
module_exit(lab3_exit);
