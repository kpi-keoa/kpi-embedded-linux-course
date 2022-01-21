#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/moduleparam.h>    //processing of module input parameters is provided by macros
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/hrtimer.h>
#include <linux/time.h>

MODULE_DESCRIPTION("Deferred Work Tools: Timers and Tasklets");
MODULE_AUTHOR("Roman Zherebchenko");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static int i;
static int t_cnt = 0;
static int cnt = NULL;
static int delay = NULL;
static unsigned long *arr;

static struct hrtimer first_hrtimer;
void tasklet_handler(struct tasklet_struct *data);
DECLARE_TASKLET(first_tasklet, tasklet_handler);
enum hrtimer_restart timer_handler(struct hrtimer *data);

module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "Number of cycles to be worked by the timer");
module_param(delay, int, 0);
MODULE_PARM_DESC(delay, "Delay between two timer activations");

static int __init lab2_mod_init(void)
{
        printk(KERN_INFO "[Init] Hello!\n[Init] jiffies = %lu\n", jiffies);
        
        tasklet_schedule(&first_tasklet);
        
        arr = kzalloc(cnt * sizeof(*arr), GFP_KERNEL);
        
        if ((cnt == NULL) || (cnt < 0)) {
                printk(KERN_ERR "[Init] Error, cnt must be greater than 0\n");
                goto end;
        }
        if ((delay == NULL) || (delay < 0)) {
                printk(KERN_ERR "[Init] Error, delay must be greater than 0\n");
                goto end;
        }
        if (arr == NULL) {
                printk(KERN_ERR "[Init] Error, arr = NULL\n");
                goto end;
        }

        hrtimer_init(&first_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        first_hrtimer.function = &timer_handler;
        hrtimer_start(&first_hrtimer, ms_to_ktime(delay), HRTIMER_MODE_REL);

/*
        CLOCK_MONOTONIC - a clock that measures monotonous time from some
        undefined starting point. This clock that cannot be set.

        CLOCK_MONOTONIC_COARSE - a clock that measures time faster
        than CLOCK_MONOTONIC, but with less accuracy.

        CLOCK_MONOTONIC_RAW - same as CLOCK_MONOTONIC, but gives access to raw
        hardware time.

        CLOCK_REALTIME - a system clock that measures real time.
    
        CLOCK_REALTIME_ALARM - same as CLOCK_REALTIME, but this clock
        that cannot be set.
    
        CLOCK_REALTIME_COARSE - a clock that measures real time faster
        than CLOCK_REALTIME, but with less accuracy.

        CLOCK_TAI - a system clock, which is based on the wall clock time, but
        ignores the high seconds. 

        CLOCK_BOOTTIME - same as CLOCK_MONOTONIC, but it also includes the time
        when the system is paused.
    
        CLOCK_BOOTTIME_ALARM - same as CLOCK_BOOTTIME

        CLOCK_PROCESS_CPUTIME_ID - a clock that measures the CPU time consumed
        by all threads in a given process. This clock that cannot be set.

        CLOCK_THREAD_CPUTIME_ID - processor time clocks that are specific to
        a particular thread. This clock that cannot be set.
*/

end:
        return 0;
}

static void __exit lab2_mod_exit(void)
{
        hrtimer_cancel(&first_hrtimer);
        tasklet_kill(&first_tasklet);

        printk(KERN_INFO "[Exit] jiffies = %lu\n", jiffies);

        if (t_cnt < cnt) {
                printk(KERN_WARNING "[Exit] Warning! Array isn't filled to end");
        }
        
        i = 0;
        while (i < t_cnt) {
                printk(KERN_INFO "[Exit] Сounter: %d, jiffies = %lu", i, arr[i]);
                i++;
        }
}

void tasklet_handler(struct tasklet_struct *data)
{
        printk(KERN_INFO "[Tasklet] jiffies = %lu\n", jiffies);
}

enum hrtimer_restart timer_handler(struct hrtimer *data)
{
        arr[t_cnt++] = jiffies;

        hrtimer_forward_now(&first_hrtimer, ms_to_ktime(delay));

        if (t_cnt < cnt) {
                return HRTIMER_RESTART;
        } else {
                return HRTIMER_NORESTART;
        }
}

module_init(lab2_mod_init);
module_exit(lab2_mod_exit);

