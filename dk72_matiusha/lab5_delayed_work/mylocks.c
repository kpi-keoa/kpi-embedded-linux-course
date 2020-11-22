#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/types.h>
#include <linux/workqueue.h>

//DESCRIPTION SECTION
MODULE_DESCRIPTION("kthread counter");
MODULE_AUTHOR("raibu");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

//PARAMS SECTION
static unsigned int th_val = 3, wrk_val = 5, jffdelay = 101;
module_param(th_val, int, 0000);
module_param(wrk_val, int, 0000);
module_param(jffdelay, int, 0000);
MODULE_PARM_DESC(th_val, "Number of threads");
MODULE_PARM_DESC(wrk_val, "value to increment each thread call");
MODULE_PARM_DESC(jffdelay, "delay between increments");

//MODULE SPECIFIC DECLARES
static struct task_struct *tthread, *wthread;
static struct timer_list my_timer = { 0 };

static struct delayed_work *work;

static int ttstop = 0, twstop = 0;

static LIST_HEAD(tlist);
static LIST_HEAD(wlist);

typedef struct {
    struct list_head ptr;
    typeof(jiffies) time;
} tw_timeval;

static int tthreadfn(void *data)
{
    struct list_head *pos = NULL;
    struct list_head *tmp;

    printk(KERN_INFO "thread: child process [%d] is running\n", current->pid);
    while (1) {
        if (kthread_should_stop() || ttstop) {
            pr_info("tthread was finished\n");
            list_for_each_safe(pos, tmp, &tlist)
            {
                tw_timeval *t = list_entry(pos, tw_timeval, ptr);
                printk(KERN_INFO "time = %ld", t->time);
                list_del(pos);
                kfree(t);
            }
            return 0;
        }
        // schedule();
    }
    return 0;
}

static int wthreadfn(void *data)
{
    struct list_head *pos = NULL;
    struct list_head *tmp;

    printk(KERN_INFO "thread: child process [%d] is running\n", current->pid);
    while (1) {
        if (kthread_should_stop() || twstop) {
            pr_info("wthread was finished\n");
            list_for_each_safe(pos, tmp, &wlist)
            {
                tw_timeval *t = list_entry(pos, tw_timeval, ptr);
                printk(KERN_INFO "time = %ld", t->time);
                list_del(pos);
                kfree(t);
            }
            return 0;
        }
        // schedule();
    }
    return 0;
}

void tcallback(struct timer_list *data)
{
    tw_timeval *tmp = kzalloc(sizeof(*tmp), GFP_ATOMIC);    //atomic
    tmp->time = jiffies;

    if (jiffies % th_val == 0) {
        ttstop = 1;
    } else {
        list_add_tail(&(tmp->ptr), &tlist);
        mod_timer(&my_timer, jiffies + msecs_to_jiffies(jffdelay));
    }
}

void wcallback(struct work_struct *wrk)
{
    tw_timeval *tmp = kzalloc(sizeof(*tmp), GFP_KERNEL);
    tmp->time = jiffies;

    if (jiffies % wrk_val == 0) {
        twstop = 1;
    } else {
        list_add_tail(&(tmp->ptr), &wlist);
        schedule_delayed_work(work, jffdelay);
    }
}

static int __init mod_init(void)
{
    int status = 0;
    // int i;

    //INIT PARAMS
    printk(KERN_INFO "Init: called with th_val = %d, wrk_val = %d, jffdelay = %d", th_val, wrk_val,
           jffdelay);

    if (th_val <= 0) {
        printk(KERN_ERR "Init: th_val <= 0");
        status = -EINVAL;
        goto err;
    }

    if (wrk_val <= 0) {
        printk(KERN_ERR "Init: wrk_val <= 0");
        status = -EINVAL;
        goto err;
    }

    if (jffdelay < 0) {
        printk(KERN_ERR "Init: jffdelay < 0");
        status = -EINVAL;
        goto err;
    }

    //INIT MODULE SPECIFICS

    tthread = kzalloc(sizeof(*tthread), GFP_KERNEL);    //non atomic context
    if (tthread == NULL) {
        printk(KERN_ALERT "Init: alloc failed, doing nothing");
        status = -ENOMEM;
        goto err;
    }

    wthread = kzalloc(sizeof(*wthread), GFP_KERNEL);    //non atomic context
    if (wthread == NULL) {
        printk(KERN_ALERT "Init: alloc failed, doing nothing");
        status = -ENOMEM;
        goto err;
    }

    work = kzalloc(sizeof(*work), GFP_KERNEL);    //non atomic context
    if (work == NULL) {
        printk(KERN_ALERT "Init: alloc failed, doing nothing");
        status = -ENOMEM;
        goto err;
    }

    tthread = kthread_run(tthreadfn, NULL, "tthread");
    get_task_struct(tthread);
    wthread = kthread_run(wthreadfn, NULL, "wthread");
    get_task_struct(wthread);

    INIT_DELAYED_WORK(work, wcallback);
    schedule_delayed_work(work, 0);
    timer_setup(&my_timer, tcallback, 0);
    mod_timer(&my_timer, jiffies);

err:
    return status;
}

static void __exit mod_exit(void)
{
    del_timer(&my_timer);
    cancel_delayed_work_sync(work);
    kfree(work);

    if (!ttstop) {
        kthread_stop(tthread);
        put_task_struct(tthread);
    }
    kfree(tthread);

    if (!twstop) {
        kthread_stop(wthread);
        put_task_struct(wthread);
    }
    kfree(wthread);

    printk(KERN_INFO "Exit: poka");
}

module_init(mod_init);
module_exit(mod_exit);
