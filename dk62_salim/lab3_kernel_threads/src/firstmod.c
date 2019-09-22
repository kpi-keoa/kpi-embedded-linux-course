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
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");// this affects the kernel behavior

static int incr_val;
static int kth_num;

int *cnt;
atomic_t *arg;

module_param(incr_val, int, 0);
MODULE_PARM_DESC(incr_val, "Value by which 'cnt' variable is incremented by threads");
module_param(kth_num, int, 0);
MODULE_PARM_DESC(kth_num, "Number of threads to create");

typedef struct struct_for_res_n {
	struct list_head list;
	int num;
} struct_for_res;
	
struct task_struct **kth_ptr;
struct_for_res *struct_ptr;
struct_for_res main_struct;

static atomic_t *new_lock(void)
{
	atomic_t *ptr = (atomic_t *)kmalloc(sizeof(atomic_t), GFP_ATOMIC);
	arch_atomic_set(ptr, 0);
	return ptr;
}

static void lock(atomic_t *arg)
{
	do {
	} while (arch_atomic_cmpxchg(arg, 0, 1) == 1);
}

static void unlock(atomic_t *arg)
{
	arch_atomic_set(arg, 0);
}

static void free_lock(atomic_t *ptr)
{
	kfree(ptr);
}

int thread_func(void *data)
{
	int i;
	for (i = 0; i < incr_val; i++) {
		lock(arg);
		*(int *)data += 1;
		unlock(arg);
		schedule();
	}
	struct_ptr = (struct_for_res *)kmalloc(sizeof(struct_for_res), 
	GFP_KERNEL);
	if (!struct_ptr) 
		goto struct_Error;
	struct_ptr->num = *(int *)data;
	list_add(&struct_ptr->list, &main_struct.list);
	printk(KERN_INFO "All calculated #%d\n", struct_ptr->num);
	do_exit(1);

	struct_Error: printk(KERN_ERR "kmalloc didn't allocate memory for struct pointer!\n"); 
	do_exit(1);
}


static int __init kmod_init(void)
{
	INIT_LIST_HEAD(&main_struct.list);
	arg = new_lock();
	if(!arg) 
		goto lock_Error;
	cnt = (int *)kmalloc(sizeof(int), GFP_KERNEL);
	if (!cnt) 
		goto cnt_Error;
	*cnt = 0;
	kth_ptr = (struct task_struct **)kmalloc(sizeof(struct task_struct *) * 
	kth_num, GFP_KERNEL);
	if (!kth_ptr) 
		goto kth_Error;	
	int i;
	for (i = 0; i < kth_num; i++) {
		kth_ptr[i] = kthread_run(thread_func, (void *)cnt, "thread_%d", 
		i);
	}
	return 0;
	
	
	lock_Error: printk(KERN_ERR "kmalloc didn`t allocate memory for 'lock'!\n");
	cnt_Error: printk(KERN_ERR "kmalloc didn`t allocate memory for cnt variable!\n"); 
	kth_Error:  printk(KERN_ERR "kmalloc didn`t allocate memory for threads pointer!\n"); 
	return ENOMEM;
}

static void __exit kmod_exit(void)
{ 
	printk(KERN_NOTICE "Counter value is #%d!\n", *cnt);
	struct list_head *head_ptr;
	struct list_head *tmp;	
	list_for_each_safe(head_ptr, tmp, &(main_struct.list)) {
		struct_ptr = list_entry(head_ptr, struct_for_res, list);
		printk(KERN_NOTICE "Deleting list element #%d!\n", 
		struct_ptr->num);
		list_del(head_ptr);
		kfree(struct_ptr);		
	}
	if(cnt)
		kfree(cnt);
	if(arg)
		free_lock(arg);
	printk(KERN_NOTICE "Module unloaded!\n" ); 
}

module_init(kmod_init);
module_exit(kmod_exit);

