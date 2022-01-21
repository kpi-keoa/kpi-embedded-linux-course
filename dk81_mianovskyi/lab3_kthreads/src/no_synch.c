#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>   // where jiffies and its helpers reside
#include <linux/slab.h>
#include <linux/list.h>   
#include <linux/kthread.h>   
#include <linux/sched/task.h>

MODULE_DESCRIPTION("Threads and lists");
MODULE_AUTHOR("trueDKstudent");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static int status = 0;
unsigned long glob_var = 0;
static int thread_num = 1;
static int thread_inccnt = 10;
static unsigned long thread_delay = 1;

static LIST_HEAD(glob_var_list);

static struct task_struct **thread = NULL;

struct list_node {
	struct list_head list;
	unsigned long glob_var_val;
};

module_param(thread_num, int, 0);
MODULE_PARM_DESC(thread_num, "Number of threads");
module_param(thread_inccnt, int, 0);
MODULE_PARM_DESC(thread_inccnt, "Number of incrementations of glob_var");
module_param(thread_delay, ulong, 0);
MODULE_PARM_DESC(thread_delay, "Delay between two incrementations");

static int thread_func(void *data)
{
	struct list_node *n = kmalloc(sizeof(*n), GFP_KERNEL);
	int cnt = thread_inccnt;	

	INIT_LIST_HEAD(&n->list);
	while (!kthread_should_stop() && cnt--) {
		n->glob_var_val = ++(*(volatile int *)data);
		schedule_timeout_uninterruptible(msecs_to_jiffies(thread_delay));
	}
	
	list_add_tail(&(n->list), &glob_var_list);
	
	return 0;
}

static int __init thirdmod_init(void)
{
	int i;

        pr_info("Modue %s is installed!\n", module_name(THIS_MODULE));
	pr_info("init glob_var = %lu\n", glob_var);

	if (thread_num <= 0) {
		pr_err("Error: invalid number of threads!\n");
		goto init_final;
	}
	if (thread_inccnt <= 0 ) {
		pr_err("Error: invalid number of iterations!\n");
		goto init_final;
	}
	if (thread_delay < 0 ) {
		pr_err("Error: invalid delay value!\n");
		goto init_final;
	}

	thread = kmalloc(thread_num*sizeof(**thread), GFP_KERNEL);
	if (NULL == thread) {
		pr_err("Error: cannot allocate memory for threads!\n");
		status = -ENOMEM;
		goto init_final;
	}

	for (i = 0; i < thread_num; i++) {
		thread[i] = kthread_run(thread_func, &glob_var, "thread-%d", i);

		if (IS_ERR(thread[i])) {
			pr_err("Error: thread-%d was not created!\n", i);
			thread[i] = NULL;
		} else {
			pr_info("thread-%d was created suceccfuly!\n", i);
		}
		get_task_struct(thread[i]);
	}
init_final:
        return status;
}

static void __exit thirdmod_exit(void)
{
	int i;
	struct list_node *pos = NULL;
	struct list_node *n;
	
	if (NULL == thread)
		goto exit_final;

	for (i = 0; i < thread_num; i++) {
        	if (NULL == thread[i])
			goto skip;

		kthread_stop(thread[i]);
		put_task_struct(thread[i]);
skip:
		pr_info("thread-%d was stoped\n", i);
	}
	kfree(thread);

	pr_info("final glob_var = %lu\n", glob_var);

	list_for_each_entry_safe(pos, n, &glob_var_list, list)
	{
		pr_info("glob_var = %lu\n", pos->glob_var_val);
		list_del(&(pos->list));
		kfree(pos);
	}
	
exit_final:
	pr_info("Modue %s is removed\n", module_name(THIS_MODULE));

}

module_init(thirdmod_init);
module_exit(thirdmod_exit);
