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
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/types.h>

MODULE_DESCRIPTION("kthread counter");
MODULE_AUTHOR("raibu");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static unsigned long glob_var = 0;

static unsigned int thread_num = 0, thread_inccnt = 0, thread_delay = 0;
module_param(thread_num, int, 0000);
module_param(thread_inccnt, int, 0000);
module_param(thread_delay, int, 0000);
MODULE_PARM_DESC(thread_num, "Number of threads");
MODULE_PARM_DESC(thread_inccnt, "value to increment each thread call");
MODULE_PARM_DESC(thread_delay, "delay between increments");
static struct task_struct **threads;
static LIST_HEAD(vlist);
static atomic_t __lock = ATOMIC_INIT(0);

static void lock(atomic_t *lockval)
{
    while (atomic_xchg(lockval, 1))
        ;
}

static void unlock(atomic_t *lockval)
{
    atomic_set(lockval, 0);
}

typedef struct __timeval {
    struct list_head ptr;
    typeof(jiffies) time;
} timeval;

static int thread(void *data)
{
    int i;

    printk(KERN_INFO "thread: child process [%d] is running\n", current->pid);
    for (i = 0; i < thread_inccnt; ++i) {
        timeval *t = kzalloc(sizeof(*t), GFP_KERNEL);
        t->time = jiffies;

        lock(&__lock);
        list_add_tail(&(t->ptr), &vlist);
        ++(*(unsigned long *)data);
        unlock(&__lock);

        msleep(thread_delay);
        if (kthread_should_stop()) {
            return 0;
        }
    }
 
    printk(KERN_INFO "thread: child process [%d] is completed\n", current->pid);
    return 0;
}

static int __init mod_init(void)
{
    int status = 0;
    int i;
    printk(KERN_INFO "Init: called with thread_num = %d, thread_inccnt = %d, thread_delay = %d",
           thread_num, thread_inccnt, thread_delay);

    if (thread_num <= 0) {
        printk(KERN_ERR "Init: thread_num <= 0");
        status = -EINVAL;
        goto err;
    }

    if (thread_inccnt < 0) {
        printk(KERN_ERR "Init: thread_inccnt < 0");
        status = -EINVAL;
        goto err;
    }

    if (thread_delay < 0) {
        printk(KERN_ERR "Init: thread_delay < 0");
        status = -EINVAL;
        goto err;
    }

    threads = kzalloc(thread_num * sizeof(*threads), GFP_KERNEL);    //non atomic context
    if (threads == NULL) {
        printk(KERN_ALERT "Init: alloc failed, doing nothing");
        status = -ENOMEM;
        goto err;
    }

    for (i = 0; i < thread_num; ++i) {
        threads[i] = kthread_run(thread, &glob_var, "th%d", i);
        get_task_struct(threads[i]);
    }

err:
    return status;
}

static void __exit mod_exit(void)
{
    int i;
    struct list_head *pos = NULL;
    struct list_head *tmp;

    for (i = 0; i < thread_num; ++i) {
        kthread_stop(threads[i]);
        put_task_struct(threads[i]);
    }
    kfree(threads);

    list_for_each_safe(pos, tmp, &vlist)
    {
        timeval *t = list_entry(pos, timeval, ptr);
        printk(KERN_INFO "time = %ld", t->time);
        list_del(pos);
        kfree(t);
    }

    printk(KERN_INFO "GLOBAL VAR: %ld", glob_var);
    printk(KERN_INFO "Exit: poka");
}

module_init(mod_init);
module_exit(mod_exit);
