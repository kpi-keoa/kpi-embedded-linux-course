#include <linux/module.h> // required by all modules
#include <linux/kernel.h> // required for sysinfo
#include <linux/init.h> // used by module_init, module_exit macros
#include <linux/jiffies.h> // where jiffies and its helpers reside
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/workqueue.h>

MODULE_DESCRIPTION("work_queue");
MODULE_AUTHOR("AlexShlikhta");
MODULE_VERSION("1.0");
MODULE_LICENSE("Dual MIT/GPL"); // this affects the kernel behavior

static int th_val = 11;
module_param(th_val, int, 0);
MODULE_PARM_DESC(th_val, "value for timer");

static int wrk_val = 11;
module_param(wrk_val, int, 0);
MODULE_PARM_DESC(wrk_val, "value for work");

static int jffdelay = 100;
module_param(jffdelay, int, 0);
MODULE_PARM_DESC(jffdelay, "delay for work and timer(in miliseconds)");

static struct task_struct *thread_work;
static struct task_struct *thread_timer;
struct delayed_work *my_work = NULL;
struct timer_list my_timer;
static bool work_thread_flag = false;
static bool timer_thread_flag = false;
static bool timer_list_empty = true;
static bool work_list_empty = true;
static LIST_HEAD(my_list_work);
static LIST_HEAD(my_list_timer);

struct _tmp_head {
        struct list_head head;
        unsigned long tmp_jiffies;
};

///////////////////////////////////////////////////////////////////////////////

int work_thread_foo(void *var)
{
        struct list_head *pos = NULL;
        struct list_head *next;

        while(!kthread_should_stop() && !work_thread_flag);
        
        if(false == work_list_empty) {
                list_for_each_safe(pos, next, &my_list_work) {
                        struct _tmp_head *tmp = list_entry(pos, struct _tmp_head, head);
                        printk(KERN_INFO "work jiffies = %ld\n", tmp->tmp_jiffies);
                        list_del(pos);
                        kfree(tmp);
                }
        }

        pr_info("work_thread was finished\n");

        return 0;
}

///////////////////////////////////////////////////////////////////////////////

int timer_thread_foo(void *var)
{
        struct list_head *pos = NULL;
        struct list_head *next;

        while(!kthread_should_stop() && !timer_thread_flag);

        if(false == timer_list_empty) {
                list_for_each_safe(pos, next, &my_list_timer) {
                        struct _tmp_head *tmp = list_entry(pos, struct _tmp_head, head);
                        printk(KERN_INFO "timer jiffies = %ld\n", tmp->tmp_jiffies);
                        list_del(pos);
                        kfree(tmp);
                }        
        }

        pr_info("timer_thread was finished\n");

        return 0;
}

///////////////////////////////////////////////////////////////////////////////

void work_foo(struct work_struct *w) 
{       
        struct _tmp_head *tmp = kzalloc(sizeof(*tmp), GFP_KERNEL);
        tmp->tmp_jiffies = jiffies;

        if(0 == tmp->tmp_jiffies % wrk_val) {

                pr_info("work thread was stopped at jiffies = %lu\n", tmp->tmp_jiffies);
                work_thread_flag = true;

                return;
        }

        list_add_tail(&(tmp->head), &my_list_work);

        if(true == work_list_empty)
                work_list_empty = false;

        schedule_delayed_work(my_work, jffdelay);
}

///////////////////////////////////////////////////////////////////////////////

void timer_foo(struct timer_list *data) 
{
        struct _tmp_head *tmp = kzalloc(sizeof(*tmp), GFP_KERNEL);
        tmp->tmp_jiffies = jiffies;

        if(0 == tmp->tmp_jiffies % th_val) {

                pr_info("timer thread was stopped at jiffies = %lu\n", tmp->tmp_jiffies);
                timer_thread_flag = true;

                return;
        }

        list_add_tail(&(tmp->head), &my_list_timer);

        if(true == timer_list_empty)
                timer_list_empty = false;

        mod_timer(&my_timer, jiffies + msecs_to_jiffies(jffdelay));
}

///////////////////////////////////////////////////////////////////////////////
//***************************INIT CALLBACK*************************************
///////////////////////////////////////////////////////////////////////////////

static int __init mod_init(void)
{

        int status = 0;

        if (th_val <= 0) {
                printk(KERN_ERR "Invalid <th_val> <= 0 :(\n");
                status = -EINVAL;
                goto final;             
        }

        if (jffdelay < 0) {
                printk(KERN_ERR "Invalid <jffdelay> < 0 :(\n");
                status = -EINVAL;
                goto final;             
        }

        if (wrk_val <= 0) {
                printk(KERN_ERR "Invalid <wrk_val> < 0 :(\n");
                status = -EINVAL;
                goto final;             
        }


        if ((thread_work = kzalloc(sizeof(*thread_work), GFP_KERNEL)) == NULL) {
                printk(KERN_ERR "Couldn't allocate a memory:(\n");
                status = -ENOMEM;
                goto final;
        }

        thread_work = kthread_run(work_thread_foo, NULL, "thread_work");
        get_task_struct(thread_work);


        if ((thread_timer = kzalloc(sizeof(*thread_timer), GFP_KERNEL)) == NULL) {
                printk(KERN_ERR "Couldn't allocate a memory:(\n");
                status = -ENOMEM;
                goto final;
        }

        thread_timer = kthread_run(timer_thread_foo, (void*)NULL, "thread_timer");
        get_task_struct(thread_timer);

        timer_setup(&my_timer, &timer_foo, 0);



        if ((my_work = kzalloc(sizeof(*my_work), GFP_KERNEL)) == NULL) {
                printk(KERN_ERR "Couldn't allocate a memory:(\n");
                status = -ENOMEM;
                goto final;
        }

        INIT_DELAYED_WORK(my_work, &work_foo);


        pr_info("Module started with parameters:\n");
        pr_info("th_val = %d\n", th_val);
        pr_info("wrk_val = %d\n", wrk_val);
        pr_info("jffdelay = %d\n\n", jffdelay);

        schedule_delayed_work(my_work, 0);      // start without delay
        mod_timer(&my_timer, jiffies);

final:
        return status;
}

///////////////////////////////////////////////////////////////////////////////
//***************************EXIT CALLBACK*************************************
///////////////////////////////////////////////////////////////////////////////
static void __exit mod_exit(void)
{       
        
        if(false == work_thread_flag) {
                kthread_stop(thread_work);
                put_task_struct(thread_work);                
        }
        kfree(thread_work);

        if(false == timer_thread_flag) {
                kthread_stop(thread_timer);
                put_task_struct(thread_timer);
        }
        kfree(thread_timer);

        del_timer(&my_timer);
        cancel_delayed_work_sync(my_work);

        kfree(my_work);

        pr_info("Goodbye:)\n");
}

module_init(mod_init);
module_exit(mod_exit);
