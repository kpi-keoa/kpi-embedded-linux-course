#include <linux/module.h>	
#include <linux/kernel.h>
#include <linux/init.h>		
#include <linux/threads.h>          
#include <linux/kthread.h>      
#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

MODULE_DESCRIPTION("Module for syncronism researching");
MODULE_AUTHOR("jayawar");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static short Mthreads = 0;
static long Ntimes = 0;

module_param(Mthreads, short, S_IRUSR);
module_param(Ntimes, long , S_IRUSR);

rwlock_t list_lock;
spinlock_t spinlock;

struct base_set {
        struct list_head list;
	unsigned long long shared_cnt;
};

struct base_set *data_chunk;

LIST_HEAD(head_list); 

void print_list(struct list_head *a_list)
{
	if (NULL == a_list)
		return;

	struct base_set *set2log;
        struct list_head *list2print;
        list_for_each(list2print, a_list) 
        {
                set2log = list_entry(list2print, struct base_set, list);
                printk(KERN_INFO "Node stored: [%lld]\n", set2log->shared_cnt);
        }
}

void destroy_list(struct list_head *a_list)
{
	if (NULL == a_list)
		return;

	struct base_set *set2log;
        struct list_head *list2del, *tmp;

	list_for_each_safe(list2del, tmp, a_list) /* Traversing the list */
        {
                set2log = list_entry(list2del, struct base_set, list);
		list_del(list2del);  
        }
}

static int kfunc(void *args)
{
	if (NULL == args) 
		return (-1);

	/* Save address of the shared variable */
	unsigned long long *local_cnt = (unsigned long long *)args;

	for (unsigned long long i = 0; i < Ntimes; i++) {
		//spin_lock( &spinlock );
		(*local_cnt)++;
		//spin_unlock( &spinlock );
	}


	if( NULL ==  (data_chunk = kmalloc(sizeof(*data_chunk), GFP_KERNEL)))
		goto alloc_error;
	

	data_chunk->shared_cnt = *local_cnt;

        write_lock(&list_lock);
	list_add(&(data_chunk->list), &head_list);  
        write_unlock(&list_lock);

	printk(KERN_INFO "PID %d thread completed", current->pid);
	do_exit(0);
	return (0);

	alloc_error:
		kfree(data_chunk);
		printk(KERN_ERR "Cannot allocate memory for new node\n");
		do_exit(0);
	return (0);
}


static int __init kthread_investigator_init(void)
{
	
	printk(KERN_INFO "Kernel thread module for synchronism investigations \n");

	short nkthreads = Mthreads;
	     	 
	static unsigned long long cnt=0;

	struct task_struct **the_kthread;
	the_kthread = kmalloc(sizeof(**the_kthread) * nkthreads, GFP_USER);
	if (NULL == the_kthread) 
		goto alloc_error;


	for (unsigned this_thread=0; this_thread<nkthreads; this_thread++) {
		the_kthread[this_thread] = kthread_create(kfunc, (void *)&cnt,
						 "thread[%d]", this_thread);
	}

	for (unsigned this_thread=0; this_thread<nkthreads; this_thread++) {
		wake_up_process(the_kthread[this_thread]);
		printk(KERN_INFO "thread[%d]", this_thread);
		the_kthread[this_thread] ? printk(KERN_INFO "is running") 
					: printk(KERN_ERR "cannot start");
	}
	kfree(the_kthread);
	return (0);

	alloc_error:
		printk(KERN_ERR "Allocate error");
		kfree(the_kthread);
		return (-1);
}

static void __exit kthread_investigator_exit(void)
{
	print_list(&head_list);
	destroy_list(&head_list);
	kfree(data_chunk);
}

module_init(kthread_investigator_init);
module_exit(kthread_investigator_exit);
