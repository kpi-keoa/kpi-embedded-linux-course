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

MODULE_DESCRIPTION("tasklets + timers");
MODULE_AUTHOR("yan latyshev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    

static int tim_cnt = 0;    
static int cnt = NULL;
static int delay = NULL;
static int i;
static unsigned long *t_array = NULL;
static ktime_t ktime;
static struct hrtimer timer1 = {0} ;

module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "ammount of cycles that timer works");

module_param(delay, int, 0);
MODULE_PARM_DESC(delay, "a delay of the timer");

void tasklet_handler(unsigned long data)
{
        printk(KERN_INFO "tasklet>> jiffies = %lu\n", jiffies);
}

enum hrtimer_restart timer_handler(struct hrtimer *data)
{
        t_array[tim_cnt] = jiffies;
    
        hrtimer_forward(&timer1,timer1.base->get_time(),ktime);

        if (++tim_cnt < cnt)
                return HRTIMER_RESTART;
        return HRTIMER_NORESTART;

}


DECLARE_TASKLET(tasklet1, tasklet_handler, (unsigned long)NULL);

static int __init testmod_init(void)
{
        int status = 0;

        if ((cnt == NULL) || (cnt < 0)) {
                printk(KERN_ERR "Invalid cnt value has to be"
                                "(unsigned)cnt > 0\n");
                status = -EINVAL;
                goto exit;
        }

        if ((delay == NULL) || (delay < 0)) {
                printk(KERN_ERR "Invalid delay value has to be"
                                "(unsigned)delay > 0\n");
                status = -EINVAL;
                goto exit;
        }

        if ((t_array = kzalloc(cnt * sizeof(*t_array), GFP_KERNEL)) == NULL) {
                printk(KERN_ERR "\n");
                status = -ENOMEM;
                goto exit;
       

        printk(KERN_INFO "Init>> jiffies is %lu\n", jiffies);

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
        
        tasklet_kill(&tasklet1);

        hrtimer_cancel(&timer1);

        if (tim_cnt < cnt)
                printk(KERN_WARNING "exit>> the array was not filled"
                        "completely");

        printk(KERN_INFO "exit>> tim_cnt: %d", tim_cnt);

        for (i = 0; i < tim_cnt; i++)
                printk(KERN_INFO "exit>> countrer number %d has the %lu"
                        "jiffies val", i, t_array[i]);

       

        printk(KERN_INFO "exit>> god save the Kernel!"
                "; jiffies = %lu\n", jiffies);
}

module_init(testmod_init);
module_exit(testmod_exit);

