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

MODULE_DESCRIPTION("synchronyzation and threads");
MODULE_AUTHOR("Nazar Volynko");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static volatile unsigned long long count_val = 0; // global variable to increment
static unsigned int thr_cnt = NULL; // ammount of threads to start
static unsigned int thr_delay = NULL; //deley in jiffies that will be implemented in threads
static unsigned int thr_incrcnt  = NULL; //increment value

static LIST_HEAD(list);

struct res {
        struct list_head ptr;
        unsigned long long c_value;
        int thr_numb;
};

struct thread_param {
        unsigned int thr_delay;
        unsigned int thr_incrcnt;
        struct completion comp;
        int thr_numb;
        struct task_struct *thread;
};

static struct thread_param *thr_dat = NULL;


module_param(thr_cnt, uint, 0);
MODULE_PARM_DESC(thr_cnt, "ammount of threads that will be started");

module_param(thr_delay, uint, 0);
MODULE_PARM_DESC(thr_delay, "a delay of thread in jiffies");

module_param(thr_incrcnt , uint, 0);
MODULE_PARM_DESC(thr_incrcnt , "a delay of the timer");

static atomic_t lock;

static void lock_spl(void)
{
	while (atomic_xchg(&lock, 1));
}

static void unlock_spl(void)
{
	atomic_set(&lock, 0);
}

static int thread_func(void * data)
{
	int i;
        struct thread_param *params = data;
        struct res *temp_n = kzalloc(sizeof(struct res),GFP_KERNEL);
        pr_info("I am %d-rd\n", params->thr_numb);
        pr_info("thread params : ammount of jiffies - %u ; increment - %u",
                params->thr_delay, params->thr_incrcnt );

        for (i = 0; i < params->thr_incrcnt; i++) {
                lock_spl();
                count_val++;
                unlock_spl();

                if(params->thr_delay != 0)
                        schedule_timeout_uninterruptible(msecs_to_jiffies(params->thr_delay));
        }

        lock_spl();
        temp_n->c_value = count_val;
        temp_n->thr_numb = params->thr_numb;

        list_add_tail(&temp_n->ptr, &list);
        unlock_spl();

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


        thr_dat = kzalloc(thr_cnt * sizeof(struct thread_param), GFP_KERNEL);
        if (thr_dat == NULL) {
                pr_err("did not manage to allocate memory for the data arr\n");
                kfree(thr_dat);
                status = -ENOMEM;
                goto exit;
        }

        int i;
        for(i = 0; i < thr_cnt; i++) {
                thr_dat[i].thr_delay = thr_delay;
                thr_dat[i].thr_incrcnt  = thr_incrcnt;
                thr_dat[i]. thr_numb = i;


                thr_dat[i].thread = kthread_run(&thread_func, &thr_dat[i],
                         "thread %d", i);

                init_completion(&thr_dat[i].comp);

        }
exit:
        return status;
}

static void __exit testmod_exit(void)
{
	int i;
        struct list_head *tmp = NULL;
        struct list_head *next = NULL;

        list_for_each_safe(tmp, next, &list)
	{
		struct res *cur = list_entry(tmp, struct res, ptr);
                pr_info("thread number %d has %llu count_val",cur->thr_numb, cur->c_value);
                list_del(tmp);
                kfree(cur);
	}

	pr_info("for the kernel !!!");

        for(i = 0; i < thr_cnt; i++) {
                pr_info("stoping the thread jiffies %lu\n", jiffies);

                wait_for_completion(&thr_dat[i].comp);
        }
}

module_init(testmod_init);
module_exit(testmod_exit);
