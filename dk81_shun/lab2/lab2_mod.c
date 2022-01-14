#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/moduleparam.h>    // required for working with module parameters
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <linux/time.h>

MODULE_DESCRIPTION("Use of hrtimers, tasklets and memory allocation");
MODULE_AUTHOR("World_conspiracy");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static int status = 0;
static unsigned long cnt = 1;
static unsigned long delay = 1;
static unsigned long *jiff_arr = NULL;
static unsigned long i = 0;
static struct hrtimer cnt_timer;

module_param(cnt, ulong, 0);
MODULE_PARM_DESC(cnt, "hrtimer ticks count");
module_param(delay, ulong, 0);
MODULE_PARM_DESC(delay, "delay between two ticks of hrtimer");

static void tasklet_handler(struct tasklet_struct *data);

DECLARE_TASKLET(task, tasklet_handler);

static void tasklet_handler(struct tasklet_struct *data)
{
    printk(KERN_INFO "Tasklet jiffies = %lu\n", jiffies);
}

static enum hrtimer_restart timer_handler(struct hrtimer *timer)
{
    jiff_arr[i] = jiffies;
    i++;

    if (i == cnt) {
        return HRTIMER_NORESTART;
    }
    hrtimer_forward_now(&cnt_timer, ms_to_ktime(delay));
    return HRTIMER_RESTART;
}

static int __init lab2_mod_init(void)
{
    printk(KERN_INFO "Init jiffies = %lu\n", jiffies);
    tasklet_schedule(&task);
    if (cnt <= 0) {
        printk(KERN_ERR "Error: invalid value of cnt!\n");
        goto final;
    }
    if (delay < 0) {
        printk(KERN_ERR "Error: invalid value of delay!\n");
        goto final;
    }

    jiff_arr = kzalloc(cnt * sizeof(*jiff_arr), GFP_KERNEL);
    if (jiff_arr == NULL) {
        printk(KERN_ERR "Error: cannot allocate memory!\n");
        status = -ENOMEM;
        goto final;
    }

    hrtimer_init(&cnt_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    cnt_timer.function = &timer_handler;
    hrtimer_start(&cnt_timer, delay, HRTIMER_MODE_REL);
final:
    return status;
}

static void __exit secondmod_exit(void)
{
    printk(KERN_INFO "Exit jiffies = %lu\n", jiffies);
    if (jiff_arr == NULL) {
        goto exit_final;
    }
    if (hrtimer_is_queued(&cnt_timer)) {
        cnt = i;
        printk(KERN_INFO "array filling stoped!");
    }
    while (i > 0) {
        printk(KERN_INFO "jiff_arr[%lu] = %lu\n",
        cnt - i, jiff_arr[cnt - i]);
        i--;
    }
    kfree(jiff_arr);
    hrtimer_cancel(&cnt_timer);
exit_final:
    tasklet_kill(&task);
}

module_init(secondmod_init);
module_exit(secondmod_exit);
