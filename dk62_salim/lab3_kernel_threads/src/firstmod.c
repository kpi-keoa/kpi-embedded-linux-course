/* Based on https://bit.ly/2kLBtD9 made by thodnev.
 * This code was implemented by:  
 *	-Olexander Ostrianko
 *	-Maxim Salim
 *	-Dana Dovzhenko
 * Task with global variable, list, threads and memory allocating was 
 * acomplished by Olexander Ostrianko and Dana Dovzhenko. 
 * 'lock()' and 'unlock()' functions were implemented by Maxim Salim and also 
 * minor fixes added by Maxim Holub. This functions used for threads 
 * syncronization.
 */

#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>	// used by kmod_init, kmod_exit macros
#include <linux/interrupt.h>
#include <linux/kthread.h>	// used by kernel threads
#include <linux/list.h>
#include <linux/slab.h>	// used for memory allocating
#include <asm/atomic.h>	// used by lock() and unlock() functions


MODULE_DESCRIPTION("Basic module demo: kthread, list, atomic");
MODULE_AUTHOR("AlexOstrianko; maksimo0; DovzhenkoD");
MODULE_VERSION("0.2");
MODULE_LICENSE("Dual MIT/GPL");// this affects the kernel behavior

static int incr_val;
static int kth_num;

int *cnt = NULL;
atomic_t *arg = NULL;

module_param(incr_val, int, 0);
MODULE_PARM_DESC(incr_val, "Value by which 'cnt' variable is incremented by threads");
module_param(kth_num, int, 0);
MODULE_PARM_DESC(kth_num, "Number of threads to create");

struct struct_for_res {
	struct list_head list;
	int num;
};
	
struct struct_for_res *struct_ptr = NULL;
struct struct_for_res main_struct;

static void *new_lock(void)
{
	atomic_t *ptr = kmalloc(sizeof *ptr, GFP_ATOMIC);
	arch_atomic_set(ptr, 0);
	return ptr;
}

static void lock(atomic_t *arg)
{
	while (arch_atomic_cmpxchg(arg, 0, 1) == 1);
}

static void unlock(atomic_t *arg)
{
	arch_atomic_set(arg, 0);
}

int thread_func(void *data)
{
	int *val = data;
	for (int i = 0; i < incr_val; i++) {
		//lock(arg);
		*val += 1;
		//unlock(arg);
		schedule();
	}
	struct_ptr = kmalloc(sizeof *struct_ptr, GFP_KERNEL);
	if (!struct_ptr) 
		goto struct_Error;
	struct_ptr->num = *val;
	list_add(&struct_ptr->list, &main_struct.list);
	printk(KERN_INFO "All calculated #%d\n", struct_ptr->num);
	do_exit(1);

	struct_Error: printk(KERN_ERR "kmalloc didn't allocate memory for struct pointer!\n"); 
	do_exit(1);
}


static int __init kmod_init(void)
{
	struct task_struct **kth_ptr = NULL;
	INIT_LIST_HEAD(&main_struct.list);
	arg = new_lock();
	if (!arg) 
		goto lock_Error;
	cnt = kmalloc(sizeof *cnt, GFP_KERNEL);
	if (!cnt) 
		goto cnt_Error;
	*cnt = 0;
	kth_ptr = kmalloc(sizeof *kth_ptr * kth_num, GFP_KERNEL);
	if (!kth_ptr) 
		goto kth_Error;	
	for (int i = 0; i < kth_num; i++) {
		kth_ptr[i] = kthread_run(&thread_func, (void *)cnt, "thread_%d", 
			i);
	}
	return 0;
	
	kth_Error:
		kfree(kth_ptr);
		kth_ptr = NULL;
	cnt_Error:
		kfree(cnt);
		cnt = NULL;
	lock_Error:  
		printk(KERN_ERR "kmalloc didn`t allocate memory!\n"); 
	return 0;
}

static void __exit kmod_exit(void)
{
	if (cnt) {
		printk(KERN_NOTICE "Counter value is #%d!\n", *cnt);
		kfree(cnt);
	} else 
		printk(KERN_NOTICE "Counter is not available!\n");
	struct list_head *head_ptr;
	struct list_head *tmp;	
	list_for_each_safe(head_ptr, tmp, &(main_struct.list)) {
		struct_ptr = list_entry(head_ptr, struct struct_for_res, list);
		printk(KERN_NOTICE "Deleting list element #%d!\n", 
			struct_ptr->num);
		list_del(head_ptr);
		kfree(struct_ptr);		
	}
	if (arg)
		kfree(arg);
	printk(KERN_NOTICE "Module unloaded!\n" ); 
}

module_init(kmod_init);
module_exit(kmod_exit);

