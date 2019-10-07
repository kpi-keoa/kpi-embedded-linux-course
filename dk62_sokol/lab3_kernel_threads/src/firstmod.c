/* Based on https://bit.ly/2kLBtD9 made by thodnev.
//Some algorithms were taken from the Ostrianko
*/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <asm/atomic.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("Yaroslav Sokol");
MODULE_VERSION("0.2.1");
MODULE_LICENSE("Dual MIT/GPL");

static int num_t;
module_param(num_t, int, 0);
MODULE_PARM_DESC(num_t, "number threads");

static int num_c;
module_param(num_c, int, 0);
MODULE_PARM_DESC(num_c, "number cycles");

struct struct_result {
	struct list_head my_list;
	int n;
};
struct struct_result my_res, *ptr_res = NULL;
struct task_struct **t = NULL;
struct list_head *iter, *iter_safe;

int *cnt = NULL;

/*//////////////////////////
//for arm
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
/*//////////////////////
//for x86
atomic_t *atom_arg = NULL;

static atomic_t *new_atomic_arg(void)
{
	atomic_t *arg = kmalloc(sizeof(*arg), GFP_KERNEL);
	arch_atomic_set(arg, 0);
	return arg;
}

static void del_atomic_arg(atomic_t *arg)
{
	kfree(arg);
}

static void lock(atomic_t *arg)
{
	while(arch_atomic_add_return(1, arg) != 1);
}

static void unlock(atomic_t *arg)
{
	arch_atomic_set(arg, 0);
}
///////////////////////////

static int thread_func(void *arg)
{
	int *c = arg;
	for(int j = 0; j < num_c; j++) {
		lock(atom_arg);
		*c += 1;
		unlock(atom_arg);
		schedule();
	}

	ptr_res = kmalloc(sizeof(*ptr_res), GFP_KERNEL);
	if(ptr_res == NULL) {
		goto Struct_Error;
	}
	ptr_res->n = *c;
	list_add(&ptr_res->my_list, &my_res.my_list);
	return 0;

	Struct_Error:
		printk(KERN_INFO "struct hasn't memory \n");
		kfree(ptr_res);
	return 0;
}

static int __init mod_init(void)
{
	printk(KERN_INFO "$$$ Start $$$\n");

	INIT_LIST_HEAD(&my_res.my_list);
	atom_arg = new_atomic_arg();

	cnt = kmalloc(sizeof(*cnt), GFP_KERNEL);
	if(cnt == NULL) {
		goto Cnt_Error;
	}
	*cnt = 0;
	t = kmalloc(sizeof(*t) * num_t, GFP_KERNEL);
	if(t == NULL) {
		goto Thread_Error;
	}

	for(int i = 0; i < num_t; i++) {
		t[i] = kthread_run(&thread_func,(void *)cnt, "_thread[%i]_", i);
	}
	return 0;

	Cnt_Error:
		printk(KERN_ERR "cnt hasn't memory\n");
		kfree(cnt);
		cnt = NULL;
	Thread_Error:
		printk(KERN_ERR "thread hasn't memory\n");
		kfree(t);
		t = NULL;
	return 0;
}


static void __exit mod_exit(void)
{
	if(cnt) {
		printk(KERN_INFO "cnt = %d\n", *cnt);
	} else {
		printk(KERN_ERR "cnt is'n available\n");
	}

	list_for_each_safe(iter, iter_safe, &(my_res.my_list)) {
		ptr_res = list_entry(iter, struct struct_result, my_list);
		printk(KERN_NOTICE "list_num = %d", ptr_res->n);
		list_del(iter);
		kfree(ptr_res);
	}

	del_atomic_arg(atom_arg);
	kfree(cnt);
	kfree(t);

	printk(KERN_INFO "$$$ Finish $$$\n");
}

module_init(mod_init);
module_exit(mod_exit);

