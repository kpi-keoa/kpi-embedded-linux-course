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
#include <asm/semaphore.h>

MODULE_DESCRIPTION("blah blah blah");
MODULE_AUTHOR("Yesych Dmytro");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static volatile unsigned long long count_val = 0; // global variable to increment
static unsigned int thr_cnt = NULL; // ammount of threads to start
static unsigned int thr_delay = NULL; //deley in jiffies that will be implemented in threads
static unsigned int thr_incrcnt  = NULL; //increment value

static LIST_HEAD(list);
static DEFINE_MUTEX(lock);

struct resaults {
        struct list_head ptr;
        typeof(count_val) c_value;
};

struct thread_params {
        unsigned int thr_delay;
        unsigned int thr_incrcnt;
        struct completion comp;
        struct task_struct *thread;
};

static struct thread_params *thr_dat = NULL;


module_param(thr_cnt, uint, 0);
MODULE_PARM_DESC(thr_cnt, "ammount of threads that will be started");

module_param(thr_delay, uint, 0);
MODULE_PARM_DESC(thr_delay, "a delay of thread in jiffies");

module_param(thr_incrcnt , uint, 0);
MODULE_PARM_DESC(thr_incrcnt , "a delay of the timer");

static int thread_func(void * data)
{
        struct thread_params *params = data;

        struct resaults *temp_n = kzalloc(sizeof(struct resaults),GFP_KERNEL);

        pr_info("thread params : jiffies to wait - %u ; increment - %u",
                params->thr_delay, params->thr_incrcnt );

        int i;
        mutex_lock(&lock);
        for (i = 0; i < params->thr_incrcnt; i++) {
                count_val++;

                if (params->thr_delay != 0)
                        schedule_timeout_uninterruptible(msecs_to_jiffies(params->thr_delay));
        }

        temp_n->c_value = count_val;

        list_add_tail(&temp_n->ptr, &list);
        mutex_unlock(&lock);

        complete(&params->comp);
                return 0;
}

static int __init testmod_init(void)
{
        int status = 0;

        if ((thr_cnt == NULL) || (thr_cnt == 0)) {
                pr_err("Invalidthr_cnt value has to be"
                                "(unsigned)thr_cnt > 0\n");
                status = -EINVAL;
                goto exit;
        }

        if (thr_delay < 0) {
                pr_err("Invalid delay value has to be"
                                "(unsigned)thr_delay >= 0\n");
                status = -EINVAL;
                goto exit;
        }

        if (thr_incrcnt  == NULL || thr_incrcnt  == 0) {
                pr_err("Invalid increment value has to be"
                                "(unsigned)thr_incrcnt  >= 0\n");
                status = -EINVAL;
                goto exit;
        }

        thr_dat = kzalloc(thr_cnt * sizeof(struct thread_params), GFP_KERNEL);
        if (thr_dat == NULL) {
                pr_err("did not managed to allocate memory for the data arr\n");
                kfree(thr_dat);
                status = -ENOMEM;
                goto exit;
        }

        int i;
        for(i = 0; i < thr_cnt; i++) {
                thr_dat[i].thr_delay = thr_delay;
                thr_dat[i].thr_incrcnt  = thr_incrcnt;

                init_completion(&thr_dat[i].comp);

                thr_dat[i].thread = kthread_run(&thread_func, &thr_dat[i],
                                "thread_%d",i);
        }
exit:
        return status;
}

static void __exit testmod_exit(void)
{

        struct list_head *tmp = NULL;
        struct list_head *next = NULL;

        list_for_each_safe(tmp, next, &list)
	{
		struct resaults *cur = list_entry(tmp, struct resaults, ptr);
                pr_info("count_val = %llu", cur->c_value);
                list_del(tmp);
                kfree(cur);
	}

        int i;
        char thr_name[TASK_COMM_LEN];
        for(i=0; i < thr_cnt; i++) {

                get_task_comm(thr_name, thr_dat[i].thread);
                pr_info("stoping the thread named: %s \t jiffies %lu\n",
                        thr_name, jiffies);

                memset(thr_name, 0, TASK_COMM_LEN);

                wait_for_completion(&thr_dat[i].comp);
        }
}

module_init(testmod_init);
module_exit(testmod_exit);
