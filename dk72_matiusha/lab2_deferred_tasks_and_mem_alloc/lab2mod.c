#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/timer.h>
#include <linux/interrupt.h>

MODULE_DESCRIPTION("interval timer module");
MODULE_AUTHOR("raibu");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static unsigned int cnt = 0, delay = 0, _cnt = 0, i = 0;
module_param(cnt, int, 0000);
module_param(delay, int, 0000);
MODULE_PARM_DESC(cnt, "Number of cycles");
MODULE_PARM_DESC(delay, "Delay between 2 cycles in the unit of system timer ticks");
static typeof(jiffies) *time_array;
static struct timer_list my_timer;

void tasklet_handler(unsigned long arg)
{
    printk(KERN_INFO "Tasklet: jiffies = %lu\n", jiffies);
}

DECLARE_TASKLET(tasklet, tasklet_handler, (unsigned long)NULL);

void timer_callback(struct timer_list *data)
{
    printk(KERN_INFO "Timer: trigger %d\n", _cnt);
    time_array[_cnt++] = jiffies;
    if (_cnt < cnt)
        mod_timer(&my_timer, jiffies + msecs_to_jiffies(delay));
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
    timer_setup(&my_timer, timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(delay));

err:
    return status;
}

static void __exit mod_exit(void)
{
    tasklet_kill(&tasklet);
    del_timer(&my_timer);

    if (_cnt != cnt)
        printk(KERN_WARNING "Exit: Unloading module dosrochno");

    for (i = 0; i < _cnt; ++i) {
        printk(KERN_INFO "Exit: arr[%d] = %lu", i, time_array[i]);
    }

    if (time_array != NULL)
        kfree((const void *)time_array);

    printk(KERN_INFO "Exit: poka");
}

module_init(mod_init);
module_exit(mod_exit);
