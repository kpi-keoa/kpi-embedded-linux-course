/* Some code is borrowed from Oleg Matyusha */

#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>

MODULE_DESCRIPTION("hrtimer");
MODULE_AUTHOR("sankodk72");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static unsigned int cnt = 0, delay = 0, _cnt = 0, i = 0;
module_param(cnt, int, 0000);
module_param(delay, int, 0000);
MODULE_PARM_DESC(cnt, "Number of cycles");
MODULE_PARM_DESC(delay, "Delay between 2 cycles in the unit of system timer ticks");
static typeof(jiffies) *time_array;
static struct hrtimer my_timer;
static ktime_t ktime;

void tasklet_handler(unsigned long arg)
{
    printk(KERN_INFO "Tasklet: jiffies = %lu\n", jiffies);
}

DECLARE_TASKLET(tasklet, tasklet_handler, (unsigned long)NULL);

enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
    printk(KERN_INFO "Timer: trigger %d\n", _cnt);
    time_array[_cnt++] = jiffies;
    hrtimer_forward_now(&my_timer, ktime);
    if (_cnt < cnt)
        return HRTIMER_RESTART;
    return HRTIMER_NORESTART;
}

static int __init mod_init(void)
{
    int status = 0;
    printk(KERN_INFO "Init: jiffies = %lu", jiffies);
    printk(KERN_INFO "Init: called with cnt = %d, delay = %d", cnt, delay);

    if (cnt <= 0) {
        printk(KERN_ERR "Init: cnt <= 0");
        status = -EINVAL;
        goto err;
    }

    time_array = kzalloc(cnt * sizeof(*time_array), GFP_KERNEL);    //non atomic context
    if (time_array == NULL) {
        printk(KERN_ALERT "Init: alloc failed, doing nothing");
        status = -ENOMEM;
        goto err;
    }

    tasklet_schedule(&tasklet);

    ktime = ktime_set(0, delay * 1000000);
    hrtimer_init(&my_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_timer.function = &timer_callback;
    hrtimer_start(&my_timer, ktime, HRTIMER_MODE_REL);

err:
    return status;
}

static void __exit mod_exit(void)
{
    tasklet_kill(&tasklet);
    hrtimer_cancel(&my_timer);

    if (_cnt != cnt)
        printk(KERN_WARNING "Exit: Unloading module error");

    for (i = 0; i < _cnt; ++i) {
        printk(KERN_INFO "Exit: arr[%d] = %lu", i, time_array[i]);
    }

    if (time_array != NULL)
        kfree((const void *)time_array);

    printk(KERN_INFO "Have a nice day!");
}

module_init(mod_init);
module_exit(mod_exit); 
