#include <linux/module.h>               //required by all modules
#include <linux/kernel.h>               //required for sysinfo
#include <linux/init.h>                 //used by module_init, module_exit macros
#include <linux/jiffies.h>              //where jiffies and its helpers reside
#include <linux/moduleparam.h>          //processing of module input parameters is provided by macros
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/list.h>

MODULE_DESCRIPTION("Kernel Threads");
MODULE_AUTHOR("Roman Zherebchenko");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");         // this affects the kernel behavior

static int i;
static volatile unsigned long glob_var = 0;
static unsigned int thread_num;
static unsigned int thread_inccnt;
static unsigned int thread_delay;

module_param(thread_num, uint, 0);
MODULE_PARM_DESC(thread_num, "Number of threads for simultaneous execution");
module_param(thread_inccnt, uint, 0);
MODULE_PARM_DESC(thread_inccnt, "Number of times each thread increments the "
                                "glob_var variable");
module_param(thread_delay, uint, 0);
MODULE_PARM_DESC(thread_delay, "Delay between increments of the glob_var "
                                "variable by each thread");

static LIST_HEAD(list);

struct v_list {
        struct list_head head;
        unsigned long cnt_val;
};

static struct task_struct **thread_dat;

static int thread_func(void *var);

static int __init lab3_no_sync_init(void)
{
        printk(KERN_INFO "[Init] Hello!\n[Init] jiffies = %lu\n", jiffies);

        thread_dat = kzalloc(thread_num * sizeof(*thread_dat), GFP_KERNEL);

        if (thread_num == NULL || thread_num == 0) {
                printk(KERN_ERR "[Init] Error, thread_num must be "
                                "greater than 0\n");
                goto end;
        }
        if (thread_inccnt  == NULL || thread_inccnt  == 0) {
                printk(KERN_ERR "[Init] Error, thread_inccnt must be "
                                "greater than 0\n");
                goto end;
        }
        if (thread_delay < 0) {
                printk(KERN_ERR "[Init] Error, thread_delay must be greater "
                                "or equal to 0\n");
                goto end;
        }
        if (thread_dat == NULL) {
                printk(KERN_ERR "[Init] Error, thread_dat = NULL\n");
                goto end;
        }

        i = 0;
        while (i < thread_num) {
                thread_dat[i] = kthread_run(thread_func, (void*)(&glob_var), 
                        "[Init] Thread %d", i);
                get_task_struct(thread_dat[i]);
                i++;
        }

end:
        return 0;
}

static void __exit lab3_no_sync_exit(void)
{
        struct list_head *tmp;
        struct list_head *next;

        i = 0;
        while (i < thread_num) {
                kthread_stop(thread_dat[i]);
                put_task_struct(thread_dat[i]);
                i++;
        }

        kfree(thread_dat);

        printk(KERN_INFO "[Exit] Final glob_var = %ld\n", glob_var);

        list_for_each_safe(tmp, next, &list) {
                struct v_list *cur = list_entry(tmp, struct v_list, head);
                printk(KERN_INFO "[Exit] glob_var = %ld\n", cur -> cnt_val);
                list_del(tmp);
                kfree(cur);
        }
}

static int thread_func(void *var)
{
        int j = 0;
        struct v_list *n = kzalloc(sizeof(*n), GFP_KERNEL);

        while (j < thread_inccnt) {
                (*(typeof(glob_var)*)var)++;

                if (thread_delay != 0) {
                        schedule_timeout_uninterruptible
                        (msecs_to_jiffies(thread_delay));
                }
                if (kthread_should_stop()) {
                        break;
                }
                j++;
        }

        n -> cnt_val = (*(typeof(glob_var)*)var);
        list_add_tail(&(n -> head), &list);
        return 0;
}

module_init(lab3_no_sync_init);
module_exit(lab3_no_sync_exit);

