#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>    // require to user terminal params
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/kthread.h>	//kernel threads
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/mutex.h> // enable mutex
#include <linux/sched.h> // execution scheduling
#include <linux/sched/task.h>


//Code parts take from Oleg Matusha and Vadim Kharchuk

MODULE_DESCRIPTION("Task for lab_4");
MODULE_AUTHOR("oleksandr lavrusenko");
MODULE_VERSION("4.0");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

long global_var; //global varible

static int thread_num;
static int thread_inccnt;
static int thread_delay;
static LIST_HEAD(list);

static struct task_struct **threads;
typedef struct __timeval {
	struct list_head ptr;
	unsigned long time;
} timeval;


module_param(thread_num, int, 0);
MODULE_PARM_DESC(thread_num, "Number of threads");

module_param(thread_inccnt, int, 0);
MODULE_PARM_DESC(thread_inccnt, "Incremention number");

module_param(thread_delay, int, 0);
MODULE_PARM_DESC(thread_delay, "Amount of delay");


static int foo(void *data)
{

        unsigned long start_jiff;

        pr_info("Task is runinng!");

	int i;
        for (i = 0; i < thread_inccnt; i++) {

                timeval *t = kzalloc(sizeof(*t), GFP_KERNEL);
		start_jiff = jiffies;

                ++(*(volatile int *)data);
		schedule_timeout_uninterruptible(msecs_to_jiffies(thread_delay));
                t->time = jiffies_to_msecs(jiffies - start_jiff);
		list_add_tail(&(t->ptr), &list);

                if (kthread_should_stop())
			return 0;
        }

        pr_info("The incremention is done!");

        return 0;

}


static int __init lab4_init(void)
{
        int status = 0;

	if (thread_num <= 0) {
		pr_err("Invalid <thread_num> <= 0 :(\n");
		status = -EINVAL;
		goto final;
	}

	if (thread_delay < 0) {
		pr_err("Invalid <thread_delay> < 0 :(\n");
		status = -EINVAL;
		goto final;
	}

        if (thread_inccnt < 0) {
		pr_err("Wrong cnt\n");
		goto final;
	}

	if ((threads = kzalloc(thread_num * sizeof(*threads),
	 		       GFP_KERNEL)) == NULL)
	{
		pr_err("Couldn't allocate a memory:(\n");
		status = -ENOMEM;
		goto final;
	}

	int i;
        for (i = 0; i < thread_num; i++) {
		threads[i] = kthread_run(foo, &global_var, "thread-%d", i);
		get_task_struct(threads[i]);
	}
	return 0;


final:
	return status;
}


static void __exit lab4_exit(void)
{

	struct list_head *pos = NULL;
	struct list_head *tmp;

        pr_info("Exit jiffies is %lu\n", jiffies);

	int i;
        for (i = 0; i < thread_num; i++) {
		kthread_stop(threads[i]);
		put_task_struct(threads[i]);
	}
	kfree(threads);

	list_for_each_safe(pos, tmp, &list)
	{
		timeval *t = list_entry(pos, timeval, ptr);
		printk(KERN_INFO "Delta time = %ldms", t->time);
		list_del(pos);
		kfree(t);
	}

	pr_info("global var: %ld", global_var);
	pr_info("%s: exit\n", module_name(THIS_MODULE));

        pr_info("Ave Kernel!\n");
}


module_init(lab4_init);
module_exit(lab4_exit);
