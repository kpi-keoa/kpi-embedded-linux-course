#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>

MODULE_DESCRIPTION("hrtimers");
MODULE_AUTHOR("Sarazhynskyi Valentyn");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static int tim_cnt = 0;    // current amount of expired counters
static bool final = false;
static unsigned long *t_array = NULL;
static ktime_t ktime;
static struct hrtimer timer1 = {0};

static int cnt = 0;
module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "ammount of cycles that timer works");

static int delay = 0;
module_param(delay, int, 0);
MODULE_PARM_DESC(delay, "a delay of the timer");

void tasklet_handler(unsigned long data)
{
        pr_info("tasklet>> jiffies = %lu\n", jiffies);
}

enum hrtimer_restart timer_handler(struct hrtimer *data)
{
        t_array[tim_cnt] = jiffies;
        final = ++tim_cnt >= cnt;
        if (final)
                //from current moment
                hrtimer_forward_now(&timer1,ktime);
                return HRTIMER_NORESTART;
        return HRTIMER_RESTART;

}

DECLARE_TASKLET_OLD(tasklet1, tasklet_handler);

static int __init testmod_init(void)
{
        int status = 0;

        if ((cnt == NULL) || (cnt < 0)) {
                pr_err("Invalid cnt value has to be"
                                "(unsigned)cnt > 0\n");
                status = -EINVAL;
                goto exit;
        }

        if ((delay == NULL) || (delay < 0)) {
                pr_err("Invalid delay value has to be"
                                "(unsigned)delay > 0\n");
                status = -EINVAL;
                goto exit;
        }

        if ((t_array = kzalloc(cnt * sizeof(*t_array), GFP_KERNEL)) == NULL) {
                pr_err("\n");
                status = -ENOMEM;
                goto exit;
        } 

        pr_info("Init>> jiffies is %lu\n", jiffies);

        tasklet_schedule(&tasklet1);

        ktime = ktime_set(0, delay * 1000000); //1ms * delay 

        hrtimer_init(&timer1, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        timer1.function = &timer_handler;
        hrtimer_start(&timer1, ktime, HRTIMER_MODE_REL);

exit:
        return status;
}

static void __exit testmod_exit(void)
{
        // thus tasklet1 will be executet but not schaduled to run again
        tasklet_kill(&tasklet1);

        hrtimer_cancel(&timer1);

        if (tim_cnt < cnt)
                pr_warn("exit>> the array was not filled"
                        " completely");

        pr_info("exit>> tim_cnt: %d", tim_cnt);
        
        int i;
        for (i = 0; i < tim_cnt; i++)
                pr_info("exit>> countrer number %d has the %lu"
                        " jiffies val", i, t_array[i]);

        kfree(t_array);
        pr_info("exit>> dealoc: array is deallocated");

        pr_info("exit>> god save the Kernel!"
                "; jiffies = %lu\n", jiffies);
}

module_init(testmod_init);
module_exit(testmod_exit);

