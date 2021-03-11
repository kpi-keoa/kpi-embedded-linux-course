#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/completion.h>

MODULE_DESCRIPTION("Work queue");
MODULE_AUTHOR("Sarazhynskyi Valentyn");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static unsigned int th_val = NULL;    // ammount of threads to start
module_param(th_val, uint, 0);
MODULE_PARM_DESC(th_val, "timer value");

static unsigned int wrk_val = NULL;    //deley in jiffies that will be implemented in threads
module_param(wrk_val, uint, 0);
MODULE_PARM_DESC(wrk_val, "work value");

static unsigned int jiff_delay = NULL;    //increment value
module_param(jiff_delay, uint, 0);
MODULE_PARM_DESC(jiff_delay, "ammount of delay");

static LIST_HEAD(Wlist);
static LIST_HEAD(Tlist);

struct ress {
    struct list_head head;
    unsigned long jval;
};

struct timer_p {
    unsigned long delay;
    unsigned long kval;
};

struct work_p {
    unsigned long delay;
    unsigned long kval;
};

struct timer_list tim;
struct task_struct *tim_thread;

struct delayed_work wrk;
struct task_struct *wrk_thread;

struct timer_p tim_p;
struct work_p wrk_p;

int t_done_flag = 0;
int t_thread_en = 0;
int w_done_flag = 0;
int w_thread_en = 0;

int wrk_th_f(void *dat)
{
    struct list_head *this;
    struct list_head *next;

    while (1) {
        if (kthread_should_stop() || w_thread_en) {
            list_for_each_safe(this, next, &Wlist)
            {
                struct ress *temp = list_entry(this, struct ress, head);
                pr_info("work jiffies = %ld\n", temp->jval);
                list_del(this);
                kfree(temp);
            }
            break;
        }
    }
    w_done_flag = 1;
    return 0;
}

int tim_th_f(void *dat)
{
    struct list_head *this;
    struct list_head *next;

    while (1) {
        if (kthread_should_stop() || t_thread_en) {
            list_for_each_safe(this, next, &Tlist)
            {
                struct ress *temp = list_entry(this, struct ress, head);
                pr_info("timer jiffies = %ld\n", temp->jval);
                list_del(this);
                kfree(temp);
            }
            break;
        }
    }
    t_done_flag = 1;
    return 0;
}

void wrk_fu(struct work_struct *dat)
{
    unsigned long temp_jif = jiffies;
    struct ress *temp = kzalloc(sizeof(*temp), GFP_KERNEL);

    pr_info("wrk_p.kval %lu", wrk_p.kval);
    if ((temp_jif % wrk_p.kval) == 0) {
        w_thread_en = 1;
        return;
    }
    temp->jval = temp_jif;
    list_add_tail(&(temp->head), &Wlist);
    schedule_delayed_work(&wrk, wrk_p.delay);
}

void tim_fu(struct timer_list *dat)
{
    unsigned long temp_jif = jiffies;
    struct ress *temp = kzalloc(sizeof(*temp), GFP_ATOMIC);

    pr_info("tim_p.kval %lu", tim_p.kval);
    if ((temp_jif % tim_p.kval) == 0) {
        t_thread_en = 1;
        return;
    }
    temp->jval = temp_jif;
    list_add_tail(&(temp->head), &Tlist);
    mod_timer(&(tim), jiffies + tim_p.delay);
}

static int __init mod_init(void)
{
    int err;
    if ((th_val == NULL) || (th_val == 0)) {
        pr_err("Invalid th_val\n");
        err = -EINVAL;
        goto exit;
    }

    if (jiff_delay == NULL || jiff_delay == 0) {
        pr_err("Invalid jiff_delay\n");
        err = -EINVAL;
        goto exit;
    }

    if (wrk_val == NULL || wrk_val == 0) {
        pr_err("Invalid wrk_val\n");
        err = -EINVAL;
        goto exit;
    }

    tim_p.delay = jiff_delay;
    wrk_p.delay = jiff_delay;
    tim_p.kval = th_val;
    wrk_p.kval = wrk_val;

    tim_thread = kthread_run(tim_th_f, NULL, "tim_t");

    wrk_thread = kthread_run(wrk_th_f, NULL, "wrk_t");

    tim.expires = jiffies + jiff_delay;
    timer_setup(&tim, tim_fu, 0);
    add_timer(&tim);

    INIT_DELAYED_WORK(&wrk, &wrk_fu);
    schedule_delayed_work(&wrk, jiff_delay);

    return 0;
exit:
    return err;
}

static void __exit mod_exit(void)
{
    del_timer_sync(&tim);
    cancel_delayed_work(&wrk);

    if (!w_done_flag) {
        kthread_stop(wrk_thread);
        put_task_struct(wrk_thread);
    }
    if (!t_done_flag) {
        kthread_stop(tim_thread);
        put_task_struct(tim_thread);
    }
    kfree(tim_thread);
    kfree(wrk_thread);
}

module_init(mod_init);
module_exit(mod_exit);
