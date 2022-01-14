#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <linux/list.h>		// lists
#include <linux/kthread.h>	// threads
//#include <linux/mutex.h>
#include <linux/atomic.h>	// for own resource lock
#include <linux/sched/task.h>

/* module defs */
MODULE_DESCRIPTION("+ + + [ T H R E A D S ] + + + ");
MODULE_AUTHOR("Ruslan Sopira");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

/* arg vars */
static uint32_t t_num = 0;
module_param(t_num, uint, 0);
MODULE_PARM_DESC(t_num, "Amount of threads to launch");

static uint32_t t_inc = 0;
module_param(t_inc, uint, 0);
MODULE_PARM_DESC(t_inc, "Number of increments for each thread");

static uint32_t t_delay = 0;
module_param(t_delay, uint, 0);
MODULE_PARM_DESC(t_delay, "Delay between each increment in a thread");

/* inner vars */
static uint32_t glob_var = 0;

/* list node */
struct varlist {
	typeof(glob_var) value;
	struct list_head node;
};

static LIST_HEAD(varlist_head);

/* thread array */
static struct task_struct **thread_arr = NULL;

/* lock mechanism */

/* @i = 0 for atomic ops */
#define DEFINE_LOCK(target) static atomic_t target = ATOMIC_INIT(0);

enum MYLOCK_STATUS {
	MYLOCK_UNLOCKED = 0,
	MYLOCK_LOCKED = 1
};

static void my_lock(atomic_t *resource)
{
	/* ~ @resource <- 1 */
	while(atomic_xchg(resource, MYLOCK_LOCKED));
}

static void my_unlock(atomic_t *resource)
{
	/* @resource <- @i */
	atomic_set(resource, MYLOCK_UNLOCKED);
}

DEFINE_LOCK(mutex_var);
DEFINE_LOCK(mutex_list);

/* THREAD DO */
static int thread_func(void *arg)
{
	uint32_t i = 0;
	typeof(glob_var) *var = arg;
	
	/* inc glob_var & add a node to a list*/
	
	struct varlist *new_node = kzalloc(sizeof(*new_node), GFP_KERNEL);
	if (NULL == new_node) {
		pr_err("%s: could not allocate memory for a list node\n",
			module_name(THIS_MODULE));
			
		return -1;
	}

	INIT_LIST_HEAD(&new_node->node);

	for (i = 0; i < t_inc; i++) {
		my_lock(&mutex_var);
		new_node->value = (*var)++;
		my_unlock(&mutex_var);

		schedule_timeout_uninterruptible(msecs_to_jiffies(t_delay));
		
		if (kthread_should_stop())
			break;
	}

	my_lock(&mutex_list);
	list_add_tail(&new_node->node, &varlist_head);
	my_unlock(&mutex_list);

	return 0;
}

/* INIT FUNC */
static int __init lab3_deflock_init(void)
{
	uint32_t i = 0;
	
	/* print arguments */
	pr_info("%s: nt = %u, ni = %u, dl = %u\n", module_name(THIS_MODULE),
		t_num, t_inc, t_delay);
	
	/* initialization */
	if (0 == t_inc) {
		pr_warn("%s: amount of increments is 0, expected >= 1\n",
		module_name(THIS_MODULE));
	}
	
	if (0 == t_num) {
		pr_err("%s ERR: amount of threads is 0, aborting\n",
		module_name(THIS_MODULE));
		goto ll_err;
	}
	
	thread_arr = kmalloc(sizeof(**thread_arr) * t_num, GFP_KERNEL);
	if (NULL == thread_arr) {
		pr_err("%s ERR: could not allocate enough memory for threads\n",
		module_name(THIS_MODULE));
		goto ll_err;
	}
	
	/* thread creation */
	for (i = 0; i < t_num; i++) {
		thread_arr[i] = kthread_run(thread_func, &glob_var, "%s-t[%u]",
		module_name(THIS_MODULE), i);
		
		if (IS_ERR(thread_arr[i])) {
			thread_arr[i] = NULL;
			pr_err("%s-t[%u] ERR: thread creation error\n",
			module_name(THIS_MODULE), i);
		}
		
		get_task_struct(thread_arr[i]);
	}
	
ll_err:
	return 0;
}

/* EXIT FUNC */
static void __exit lab3_deflock_exit(void)
{
	uint32_t i = 0;
	uint32_t var_expected = t_num * t_inc;
	struct varlist *it, *temp;
	
	/* do cleanup and print */
	if (NULL == thread_arr) {
		pr_warn("%s: no threads were allocated\n",
		module_name(THIS_MODULE));
		goto ll_end;
	}
	
	for (i = 0; i < t_num; i++) {
		kthread_stop(thread_arr[i]);
		put_task_struct(thread_arr[i]);
	}
	kfree(thread_arr);
	
	/* print stuff */
	if (glob_var == var_expected) {
		pr_info("%s: glob_var = %u\n",
		module_name(THIS_MODULE), glob_var);
	} else {
		pr_info("%s: glob_var = %u, expected = %u\n",
		module_name(THIS_MODULE), glob_var, var_expected);
	}
	
	i = 0;
	list_for_each_entry_safe(it, temp, &varlist_head, node) {
		pr_info("%s: node[%u]->glob_var = %u\n",
			module_name(THIS_MODULE), i, it->value);
			
		list_del(&it->node);
		kfree(it);
		
		i++;
	}
	
ll_end:
	return;
}

module_init(lab3_deflock_init);
module_exit(lab3_deflock_exit);
