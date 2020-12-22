#include <linux/module.h>	// required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/kthread.h>	//kernel threads
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/sched/task.h>
#include <linux/types.h>

MODULE_DESCRIPTION("blah blah blah");
MODULE_AUTHOR("Yesych Dmytro");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static unsigned int th_val = 0; // ammount of threads to start
static unsigned int wrk_val = 0; //deley in jiffies that will be implemented in threads
static unsigned int jiff_delay  = 0; //increment value

module_param(th_val, uint, 0);
MODULE_PARM_DESC(th_val, "timer value");

module_param(wrk_val, uint, 0);
MODULE_PARM_DESC(wrk_val, "work value");

module_param(jiff_delay , uint, 0);
MODULE_PARM_DESC(jiff_delay , "ammount of jiffies for work and timer to wait");

static LIST_HEAD(Work_list);
static LIST_HEAD(Timer_list);
static DEFINE_MUTEX(lock);

struct resaults {
        struct list_head ptr;
        unsigned long jiff_value;
};

struct tim_params {
        unsigned long delay;
        unsigned long e_val;
        struct timer_list tim;
        struct task_struct *thread_tim;
};

struct wrk_params {
        unsigned long delay;
        unsigned long e_val;
        struct delayed_work work;
        struct task_struct *thread_wrk;
};

struct tim_params *t_params;
struct wrk_params *w_params;

bool wrk_thread_fl;
bool tim_thread_fl;
bool tim_comp_f;
bool wrk_comp_f;

int work_thread_f(void *dat)
{
        struct list_head* cur;
        struct list_head* next;

        while(!kthread_should_stop() && !wrk_thread_fl);

        mutex_lock(&lock); // this lock is used for log to be readable
        pr_info("work_thread is going to be finished\n"
                "list of unaliquot jiffies:\n");

        list_for_each_safe(cur, next, &Work_list) {
                struct resaults *temp = list_entry(cur, struct resaults, ptr);
                printk(">>work jiffies = %ld\n", temp->jiff_value);

                list_del(cur);
                kfree(temp);
        }
        mutex_unlock(&lock);

        wrk_comp_f = true;
        return 0;
}

int timer_thread_f(void *dat)
{
        struct list_head* cur;
        struct list_head* next;

        while(!kthread_should_stop() && !tim_thread_fl);

        mutex_lock(&lock);
        printk("timer_thread is going to be finished\n"
                "list of unaliquot jiffies:\n");


        list_for_each_safe(cur, next, &Timer_list) {
                struct resaults *temp = list_entry(cur, struct resaults, ptr);
                printk(">>timer jiffies = %ld\n", temp->jiff_value);
                list_del(cur);
                kfree(temp);
        }
        mutex_unlock(&lock);

        tim_comp_f = true;
        return 0;
}

void work_f (struct work_struct *dat) {
        struct wrk_params* wrk_par = container_of((struct delayed_work *)dat, struct wrk_params, work);
        struct resaults* temp = kzalloc(sizeof(*temp), GFP_KERNEL);
        unsigned long cach;

        cach = jiffies;
        if (!(cach % wrk_par->e_val)) {

                wrk_thread_fl = true;
                return;
        }

        temp->jiff_value = cach;
        list_add_tail(&(temp->ptr), &Work_list);
        schedule_delayed_work(&(wrk_par->work), wrk_par->delay);
}


void timer_f (struct timer_list *dat) {
        struct tim_params* tim_par = container_of(dat, struct tim_params, tim);
        struct resaults* temp = kzalloc(sizeof(*temp), GFP_ATOMIC);
        unsigned long cach;

        cach = jiffies;
        if (!(cach % tim_par->e_val)) {
                tim_thread_fl = true;
                return;
        }

        temp->jiff_value = cach;
        list_add_tail(&(temp->ptr), &Timer_list);
        mod_timer(&(tim_par->tim), jiffies + tim_par->delay);
}

static int __init testmod_init(void)
{
        int status = 0;


        if ((th_val == NULL) || (th_val == 0)) {
                pr_err("Invalid th_val\n");
                status = -EINVAL;
                goto exit;
        }

        if (jiff_delay == 0 || jiff_delay == NULL) {
                pr_err("Invalid jiff_delay\n");
                status = -EINVAL;
                goto exit;
        }

        if (wrk_val  == NULL || wrk_val == 0) {
                pr_err("Invalid wrk_val\n");
                status = -EINVAL;
                goto exit;
        }

        t_params = kzalloc( sizeof(struct tim_params), GFP_KERNEL);
        if (t_params == NULL) {
                pr_err("did not managed to allocate memory for the tim_data\n");
                status = -ENOMEM;
                goto exit;
        }

        w_params = kzalloc( sizeof(struct wrk_params), GFP_KERNEL);
        if (w_params == NULL) {
                pr_err("did not managed to allocate memory for the wrk_data\n");
                kfree(t_params);
                status = -ENOMEM;
                goto exit;
        }

        t_params->delay = jiff_delay;
        w_params->delay = jiff_delay;

        t_params->e_val = th_val;
        w_params->e_val = wrk_val;

        tim_comp_f = false;
        tim_thread_fl = false;
        t_params->thread_tim = kthread_run(timer_thread_f, NULL,
                                "tim_thread");

        wrk_comp_f = false;
        wrk_thread_fl = false;
        w_params->thread_wrk = kthread_run(work_thread_f, NULL,
                                "wrk_thread");

        t_params->tim.expires = jiffies + jiff_delay;
        timer_setup(&(t_params->tim), timer_f, 0);
        add_timer(&(t_params->tim));

        INIT_DELAYED_WORK(&(w_params->work), &work_f);
        schedule_delayed_work(&(w_params->work), jiff_delay);

exit:
        return status;
}

static void __exit testmod_exit(void)
{
        // char thr_name[TASK_COMM_LEN];
        // get_task_comm(thr_name, thr_dat[i].thread);
        printk("god save the kernel");
        del_timer_sync(&(t_params->tim));
        cancel_delayed_work(&(w_params->work));

        printk("god save the kernel");
        while (!(wrk_comp_f && tim_comp_f));

        kfree(t_params->thread_tim);
        kfree(t_params);

        kfree(w_params->thread_wrk);
        kfree(w_params);

}

module_init(testmod_init);
module_exit(testmod_exit);
