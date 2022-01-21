#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>   // where jiffies and its helpers reside
#include <linux/slab.h>
#include <linux/list.h>   // list
#include <linux/kthread.h>    // thread
#include <linux/mutex.h>
#include <linux/sched/task.h>

MODULE_DESCRIPTION("module kthread");
MODULE_AUTHOR("Vitalii Yurchenko");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static unsigned long thread_num = 4;
static unsigned long thread_inccnt = 4;
static unsigned long thread_delay = 100;

module_param(thread_num, ulong, 0000);
module_param(thread_inccnt, ulong, 0000);
module_param(thread_delay, ulong, 0000);
MODULE_PARM_DESC(thread_num, "Parameter saves number");
MODULE_PARM_DESC(thread_inccnt, "Parameter saves increment");
MODULE_PARM_DESC(thread_delay, "Parameter saves delay");

DEFINE_MUTEX(mutex_list);

static unsigned long glob_var;

struct var_list {
	typeof(glob_var) var;
	struct list_head mylist;
};

static struct task_struct **my_thread = NULL;

static LIST_HEAD(mylist_head);

static int thread_fun(void *i_param)
{
	unsigned long i;
	typeof(glob_var) *var = i_param;

	struct var_list *new_var = kzalloc(sizeof(*new_var), GFP_KERNEL);
	if (NULL == new_var) {
		pr_err("%s: new_var: allocate a memory\n", module_name(THIS_MODULE));
		return 0;
	}

	INIT_LIST_HEAD(&new_var->mylist);

	for (i = 0; i < thread_inccnt; i++) {
		new_var->var = ++(*var);

		schedule_timeout_uninterruptible(msecs_to_jiffies(thread_delay));

		if (kthread_should_stop())
			break;
	}

	mutex_lock(&mutex_list);
	list_add_tail(&new_var->mylist, &mylist_head);
	mutex_unlock(&mutex_list);
	return 0;
}

static int __init mymodule_init(void)
{
	unsigned long i;

	pr_info("%s: start\n", module_name(THIS_MODULE));

	glob_var = 0;

	if (0 == thread_delay) {
		pr_warn("%s: thread_delay == 0\n", module_name(THIS_MODULE));
	}

	if (0 == thread_inccnt) {
		pr_warn("%s: thread_inccnt == 0\n", module_name(THIS_MODULE));
	}

	if (0 == thread_num) {
		pr_err("%s: thread_num == 0\n", module_name(THIS_MODULE));
		goto end;
	}

	my_thread = kmalloc(sizeof(**my_thread)*thread_num, GFP_KERNEL);
	if (NULL == my_thread) {
		pr_err("%s: ERROR: allocate a memory\n", module_name(THIS_MODULE));
		goto end;
	}

	for (i = 0; i < thread_num; i++) {
		my_thread[i] = kthread_run(thread_fun, (void *)(&glob_var),
				"%s-thread%lu", module_name(THIS_MODULE), i);
		if (IS_ERR(my_thread[i])) {
			pr_err("Thread creation error %s\n",
					PTR_ERR(my_thread[i]) == -ENOMEM ? "ENOMEM" : "EINTR");
		my_thread[i] = NULL;
		}
		get_task_struct(my_thread[i]);
	}
end:
	return 0;
}

static void __exit mymodule_exit(void)
{
	int i;
	struct var_list *pos, *n;

	pr_info("%s: glob_var = %lu\n", module_name(THIS_MODULE), glob_var);

	if (NULL == my_thread) {
		pr_warn("%s: my_thread memory not allocated\n", module_name(THIS_MODULE));
		goto end;
	}

	for (i = 0; i < thread_num; i++) {
		kthread_stop(my_thread[i]);
		put_task_struct(my_thread[i]);
	}
	kfree(my_thread);

	list_for_each_entry (pos, &mylist_head, mylist) {
		pr_info("%s: mylist_var = %lu\n",
				module_name(THIS_MODULE), pos->var);
	}

	list_for_each_entry_safe (pos, n, &mylist_head, mylist) {
		list_del(&pos->mylist);
		kfree(pos);
	}
end:
	return;
}

module_init(mymodule_init);
module_exit(mymodule_exit);

