
// Made by CYB3RSP1D3R  

#include <linux/module.h>	// required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>	// used by module_init, module_exit macros
#include <linux/cpumask.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/list.h>	

MODULE_DESCRIPTION("Module for parallel counting.");
MODULE_AUTHOR("CYB3RSP1D3R");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL"); // this affects the kernel behavior

#define THREAD_ALLOC_ERROR 0x0001;
#define LIST_ALLOC_ERROR 0x0002;
#define THREAD_DATA_ALLOC_ERROR 0x0004;

#define error_decected(error_index, message) ((error_index) & (message))

int global_inc = 0;

static unsigned int inc_times = 1000;
module_param(inc_times, unsigned int, 0);
MODULE_PARM_DESC(inc_times, "How many times variable should be incremented.");

static unsigned int threads_num = 1;
module_param(threads_num, unsigned int, 0);
MODULE_PARM_DESC(threads_num, "How many cores should do the incrementation.");

struct kern_list {
    struct list_head lhead;
    int counter_val;
    int thread_index;
};

struct list_head head;
struct list_head *plist = NULL;
static struct task_struct **kthreads = NULL;
static int *thread_index = NULL;
static atomic_t *varlock = NULL;

LIST_HEAD(&head);

static atomic_t *create_lock(void);
static void lock(atomic_t *variable);
static void unlock(atomic_t *variable);
int thread_func(void *limit);
static void delete_list(struct list_head **cursor, struct list_head *head);
static void print_list(struct list_head *cursor, struct list_head *head);
void clean_memory(void);

static void __exit clean_module(void);

static int __init init_mod(void)
{

    if (threads_num > num_online_cpus()) {
    
        printk(KERN_ERR "ERROR: Indicated number of processors is higher than your hardware has!\n");
        printk(KERN_INFO "This system has %d processors.\n", num_online_cpus());
        
        clean_module();
        return 1;
    
    } else {
        
        printk(KERN_INFO "Module successfully installed.\n");
    
    }

    kthreads = kmalloc(sizeof(*kthreads) * threads_num, GFP_KERNEL);

    if (kthreads == NULL) {
        error_handler(THREAD_ALLOC_ERROR);
        clean_module();
        return -ENOMEM;
    }

    thread_index = kmalloc(sizeof(*thread_index) * threads_num, GFP_KERNEL);

    if (thread_index == NULL) {
        clean_module();
        return -ENOMEM;
    }

    varlock = create_lock();

    for (int i = 0; i < threads_num; i++) {
        thread_index[i] = i;
        kthreads[i] = kthread_run(thread_func, &thread_index[i], "thread count %d", i);
    }

    return 0;
}


static atomic_t *create_lock(void)
{
	atomic_t *ptrlock = kmalloc(sizeof(*ptrlock), GFP_KERNEL);

	if (ptrlock != NULL) {
		printk(KERN_ERR "Allocation error (ptr_lock)");	
		kfree(ptrlock);
        clean_module();
   	    return -ENOMEM;
	}

	atomic_set(ptr_lock, 0);
	return ptrlock;

}

static void lock(atomic_t *variable)
{
	while(atomic_cmpxchg(variable, 0, 1));
}

static void unlock(atomic_t *variable)
{
	atomic_set(variable, 0);
}

int thread_func(void *thread_index) 
{
    lock(varlock);

    for(register int i = 0; i < inc_times; i++){
        global_inc++;
    }

	kern_list *ptr = kmalloc(sizeof(struct kern_list), GFP_KERNEL);
	
	if (ptr == NULL) {
		error_handler(LIST_ALLOC_ERROR);
        return -ENOMEM;
	}
	
	ptr->counter_val = global_inc;
    ptr->thread_index = *((int*) thread_index);

    plist = ptr->lhead;
    list_add(plist,&head);

    unlock(varlock);

	return 0;
}

void clean_memory(void)
{
    if (thread_index != NULL) {
        kfree(thread_index);
    }
    if (plist != NULL) { 
        delete_list(&plist, &head);
    }
    if (kthreads != NULL) {
        kfree(kthreads);
    }
    if(varlock != NULL) {
        kfree(varlock);
    }

    return;
}

static void delete_list(struct list_head **cursor, struct list_head *head)
{
	struct list_head *tmp;
	struct kern_list *pk_list;
	
	list_for_each_safe (&cursor, tmp,  head) {
		pk_list = list_entry(&cursor, typeof(*pk_list), lhead);

		list_del(*cursor);
		kfree(pk_list);
	}
}

static void print_list(struct list_head *cursor, struct list_head *head)
{
	struct kern_list *pk_list;

	list_for_each (cursor, head) {
		pk_list = list_entry(cursor, typeof(*pk_list), lhead);
		printk(KERN_INFO "Counter %d ticked %d\n",
			pk_list->thread_index, pk_list->counter_val);
	}
}

static void __exit clean_module(void)
{
    print_list(plist, &head);
    clean_memory();

    printk(KERN_INFO "Module successfully uninstalled.\n");
}

module_init(init_mod);
module_exit(clean_module);
