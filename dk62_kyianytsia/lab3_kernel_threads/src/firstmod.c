/* Based on https://bit.ly/2kLBtD9 made by thodnev.
//Some algoritms were taken from the Yaroslav Sokol
*/
#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <asm/atomic.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("Artem Kyianytsia");
MODULE_VERSION("0.1.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior


static int val_of_threads;
module_param(val_of_threads, int, 0);
MODULE_PARM_DESC(num_t, "number threads");

static int val_of_cycles;
module_param(val_of_cycles, int, 0);
MODULE_PARM_DESC(num_c, "number cycles");

struct save_list {
	struct list_head list;
	int value;
};

struct save_list *list_pointer = NULL;
struct save_list head_list;
struct task_struct **t = NULL;
struct list_head *iter, *iter_safe;

int *count = NULL;
atomic64_t *atom_arg = NULL;

static atomic64_t *new_atomic_arg(void)
{
	atomic64_t *arg = kmalloc(sizeof(*arg), GFP_KERNEL);
	atomic64_set(arg, 0);
	return arg;
}

static void del_atomic_arg(atomic64_t *arg)
{
	kfree(arg);
}

static void lock(atomic64_t *arg)
{
	while(atomic64_add_return(1, arg) != 1);
}

static void unlock(atomic64_t *arg)
{
	atomic64_set(arg, 0);
}

int thread_func(void *data)
{
	int *func_value = data;
	for (int i = 0; i < val_of_cycles; i++){
		lock(atom_arg);
		*func_value += 1;
		unlock(atom_arg);
		schedule();
	}
	list_pointer =  kmalloc(sizeof (*list_pointer), GFP_KERNEL);
	if(list_pointer == NULL) {
		goto Struct_Error;
	}
	list_pointer->value = *func_value;
	list_add(&list_pointer->list, &head_list.list);
	return 0;

	Struct_Error:
		printk(KERN_INFO "struct hasn't memory \n");
		kfree(list_pointer);
	return 0;
}

static int __init firstmod_init(void)
{
	INIT_LIST_HEAD (&head_list.list);
	atom_arg = new_atomic_arg();
	count = kmalloc(sizeof(*count), GFP_KERNEL);
	*count = 0;
	if(count == NULL) {
		goto Count_Error;
	}
	t = kmalloc(sizeof(*t) * val_of_threads, GFP_KERNEL);
	if(t == NULL) {
		goto Thread_Error;
	}
	for (int i = 0; i < val_of_threads; i++){
		t[i] = kthread_run(&thread_func, (void *)count, "_thread[%i]_", i);
	}
	return 0;

	Count_Error:
		printk(KERN_ERR "count hasn't memory\n");
		kfree(count);
		count = NULL;
	Thread_Error:
		printk(KERN_ERR "thread hasn't memory\n");
		kfree(t);
		t = NULL;
	return 0;
}
 
static void __exit firstmod_exit(void)
{
	if(count) {
		printk(KERN_INFO "count = %d\n", *count);
	} else {
		printk(KERN_ERR "Count = NULL\n");
	}	

	list_for_each_safe(iter, iter_safe, &(head_list.list)) {
		list_pointer = list_entry(iter, struct save_list, list);
		printk (KERN_NOTICE "list value = %d", list_pointer->value);
		list_del(iter);
		kfree(list_pointer);
	}
	
	del_atomic_arg(atom_arg);
	kfree(count);
	kfree(t);

	printk(KERN_INFO "Module ended\n");
}
 


module_init(firstmod_init);
module_exit(firstmod_exit);

