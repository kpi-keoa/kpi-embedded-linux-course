#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/mm.h>

MODULE_DESCRIPTION("tasklets and timers");
MODULE_AUTHOR("Sarazhynskyi Valentyn");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static int tim_cnt = 0;    // current amount of expired counters
static int cnt = NULL;
static int delay = NULL;
static int i;
static unsigned long *t_array;

static struct timer_list timer1;

module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "ammount of cycles that timer works");

module_param(delay, int, 0);
MODULE_PARM_DESC(delay, "a delay of the timer");

void tasklet_handler(unsigned long data)
{
        printk(KERN_INFO "tasklet>> jiffies = %lu\n", jiffies);
}

void timer_handler(struct timer_list *data)
{
        if (tim_cnt < cnt) {
                t_array[tim_cnt++] = jiffies;
                mod_timer(&timer1, jiffies + msecs_to_jiffies(delay));
        }
}

DECLARE_TASKLET_OLD(tasklet1, tasklet_handler);

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
        }

        printk(KERN_INFO "Init>> jiffies is %lu\n", jiffies);

        tasklet_schedule(&tasklet1);

        timer1.expires = jiffies + msecs_to_jiffies(delay);
        timer_setup(&timer1, timer_handler, 0);
        add_timer(&timer1);

exit:
        return status;
}

static void __exit testmod_exit(void)
{
        // thus tasklet1 will be executet but not schaduled to run again
        tasklet_kill(&tasklet1);

        del_timer(&timer1);

        if (tim_cnt != cnt)
                printk(KERN_WARNING "exit>> the array was not filled"
                                    "completely");

        printk(KERN_INFO "exit>> tim_cnt %d", tim_cnt);

        for (i = 0; i < tim_cnt; i++)
                printk(KERN_INFO "exit>> countrer number %d has the %lu"
                                 "jiffies val",
                       i, t_array[i]);

        kfree((const void *)t_array);
        printk(KERN_INFO "exit>> dealoc: array is deallocated");

        printk(KERN_INFO "exit>> god save the Kernel!"
                         "; jiffies = %lu\n",
               jiffies);
}

module_init(testmod_init);
module_exit(testmod_exit);
