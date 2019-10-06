/*
* Based on MaksHolub's threadmod.c https://bit.ly/2llv9Tc
*/


#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/list.h>		// used by linked list
#include <linux/slab.h>		// used by kmalloc()
#include <linux/kthread.h>	// used by kernel threads
#include <asm/atomic.h>		// used by atomic instruction


// LOCK - is defined through Makefile
#ifdef LOCK
        #define SPINLOCK 1
#else
        #define SPINLOCK 0
#endif


MODULE_DESCRIPTION("threads, lists, synchronization tests");
MODULE_AUTHOR("rtchoke");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

/*
* lock - function which defends shared region(counter) from intruding of alien threads
* @lock - address of lock variable 
*/
static inline void lock(atomic_t *lock);
/*
* unlock - function unlocks shared region, so alien threads could intrude the foxhole..
* @lock - address of lock variable
*/
static inline void unlock(atomic_t *lock);

/*
* thr_amnt - variable which declares amount of running threads
*/
static int thr_amnt = 3;
/*
* inc_cnt - declares how many times should thread increment glob_cnt.
*/
static int inc_cnt = 1000000;
/*
* glob_cnt - variable which is incremented by threads (actually it's the foxhole)
*/
static int *glob_cnt;
atomic_t my_lock = ATOMIC_INIT(0);
module_param(thr_amnt, int, 0);
module_param(inc_cnt, int, 0);

/**
 * LIST_HEAD provide initialization of kernel linked list
 */
LIST_HEAD(head_list);

/**
 * struct k_list - struct of element kernel linked list.
 * @test_list: 	list_head type, which neccessary to provide kernel 
 *		linked list mechanism 				
 * @data: 	Current count value of node.
 * @thr_num: used to identify of node
 *          
 * This struct neccessary to work with kernel linked list
 * It contains a special variable test_list, 
 * which is an instance of a struct kernel linked list.
 */
struct k_list {
        struct list_head list;
        int thr_num;
        int data;
};
struct k_list *klist;

/*
* thread_func - it's a thread function which is used by every called thread
* (Battlefield for the foxhole. The one who has a magic key from lock may be called a winner)
* @args - it's void pointer to glob_cnt
*/
static int thread_func(void *args)
{       
        if(SPINLOCK) {
                printk(KERN_INFO "Running module with lock");
                lock(&my_lock);
        } else 
                printk(KERN_INFO "Running module without lock");

        printk(KERN_INFO "args_cnt before loop : %d", *(int*)args);
        for(int i = 0; i < inc_cnt; i++)
                (*(int*)args)++;
        printk(KERN_INFO "args_cnt after loop: %d", *(int*)args);
        klist = kmalloc(sizeof(struct k_list), GFP_KERNEL);       
        if(NULL == klist) {
                printk(KERN_ERR "Can't allocate memory for list");
                goto errlist;
        }

        klist->data = *(int*)args;
        klist->thr_num = (*(int*)args) / inc_cnt;
        list_add(&klist->list, &head_list);

        if(SPINLOCK)
                unlock(&my_lock);
        return 0;

        errlist:
                kfree(klist);
                return -ENOMEM;
}
/*
* print_list - printing information from list about data which every thread counted
*/
static void print_list(void)
{
        struct k_list *_list = NULL;
        struct list_head *entry_ptr = NULL;
        list_for_each(entry_ptr, &head_list) {
                _list = list_entry(entry_ptr, struct k_list, list);
                printk(KERN_INFO" thread %d has data: %d\n", _list->thr_num, _list->data);
        }
}
/*
* delete_list - deletes information from list(also delete list)
*/
static void delete_list(void)
{
        struct k_list *_list = NULL;
        struct list_head *entry_ptr = NULL, *tmp = NULL;
        list_for_each_safe(entry_ptr, tmp, &head_list) {
                _list = list_entry(entry_ptr, struct k_list, list);  
                printk(KERN_NOTICE "Deleting #%d!\n", _list->thr_num);
                list_del(entry_ptr);
                kfree(_list);
        }
        printk(KERN_INFO "Successfully deleted all nodes!\n");
}

static inline void lock(atomic_t *lock)
{
        while(atomic_cmpxchg(lock, 0, 1) == 1);
}

static inline void unlock(atomic_t *lock)
{
        atomic_set(lock, 0);
}
/*
* threads_test_init - init function where battle begins, threads are preparing for a fight (memory allocation)
*                     but after not a long fight, they're thrown away to trashcan (by deallocating memory)
*/
static int __init threads_test_init(void)
{
        glob_cnt = kmalloc(sizeof(int), GFP_KERNEL);
        if(NULL == glob_cnt) {
                printk(KERN_ERR "Can't allocate memory for counter");
                goto errcnt;
        }
        *glob_cnt = 0;
        struct task_struct **kthread_t = kmalloc(thr_amnt * sizeof(**kthread_t), GFP_KERNEL);
        if(NULL == kthread_t) {
                printk(KERN_ERR "Can't allocate memory for threads");
                goto errmem;
        }
        for (int i = 0; i < thr_amnt; i++) 
		kthread_t[i] = kthread_run(thread_func, (void*)glob_cnt, "thread%d", i);	

	kfree(kthread_t);	

	return 0;

        errcnt:
                kfree(glob_cnt);
                return 0;
        errmem:
                kfree(glob_cnt);
                kfree(kthread_t);
                return 0;


}
/*
* threads_test_exit - where the battle ends, function that beats up the results, and clears the battlefield(by deallocating memory)
*/
static void __exit threads_test_exit(void)
{
        printk(KERN_ALERT "Global counter = %d \n", *glob_cnt);
        print_list();
        delete_list();
        kfree(glob_cnt);
        printk(KERN_ALERT "EXIT!\n");
}

module_init(threads_test_init);
module_exit(threads_test_exit);
