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
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>	// used by kernel interrupt
#include <linux/completion.h>

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


static void work_handler(struct work_struct *);
static void timer_handler(struct timer_list *);
static void delete_list(struct list_head *_klist);
static void print_list(struct list_head *_klist);
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



atomic_t my_lock = ATOMIC_INIT(0);

// module_param(thr_amnt, int, 0);
// module_param(inc_cnt, int, 0);

/**
 * LIST_HEAD provide initialization of kernel linked list
 */
LIST_HEAD(tmr_list);
LIST_HEAD(wrk_list);

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
static struct k_list {
        struct list_head list;
        int thr_num;
        long long data;
};

static struct task_struct *kthread_tmr, *kthread_wrk;
static struct delayed_work my_work;
static struct timer_list my_timer;
static struct completion timer_done, work_done;
DECLARE_COMPLETION(work_done);
DECLARE_COMPLETION(timer_done);
DEFINE_TIMER(my_timer, &timer_handler);


static int thr_wrk_fnc(void *_unused)
{
        while (!completion_done(&work_done)) {
		 schedule();
   	}
        printk (KERN_INFO "Thread stopped!\n");
        lock(&my_lock);
        print_list(&wrk_list);
        unlock(&my_lock);
        return 0;
}

static int thr_tmr_fnc(void *_unused)
{
        while (!completion_done(&timer_done)) {
		 schedule();
   	}
        printk (KERN_INFO "Thread stopped!\n");
        lock(&my_lock);
        print_list(&tmr_list);
        unlock(&my_lock);
        return 0;
}

static void work_handler(struct work_struct *_unused)
{
                printk(KERN_INFO "WORK HANDLER!\n");
                if(jiffies % 11 == 0) {
                        printk(KERN_INFO "Trying to stop work\n");
                        complete(&work_done);
                        printk(KERN_INFO "Work stopped \n");
                } else {
                        struct k_list *klist_wrk = kmalloc(sizeof(*klist_wrk), GFP_ATOMIC);       
                        if(NULL == klist_wrk) {
                                printk(KERN_ERR "Can't allocate memory for list");
                                kfree(klist_wrk);
                        }
                        klist_wrk->data = jiffies;
                        klist_wrk->thr_num = 2;
                        lock(&my_lock);
                        list_add(&klist_wrk->list, &wrk_list);
                        unlock(&my_lock);
                        printk(KERN_INFO "jiffies added to work list : %lld\n", klist_wrk->data);
                        schedule_delayed_work(&my_work, 17);
                }       

}

static void timer_handler(struct timer_list *_unused)
{               
                printk(KERN_INFO "TIMER HANDLER!\n");
                if(jiffies % 11 == 0) {
                        printk(KERN_INFO "Trying to stop timer\n");
                        complete(&timer_done);
                        printk(KERN_INFO "Timer stopped \n");
                } else {
                        mod_timer(&my_timer, jiffies + 17);
                        struct k_list *klist_tmr = kmalloc(sizeof(*klist_tmr), GFP_ATOMIC);       
                        if(NULL == klist_tmr) {
                                printk(KERN_ERR "Can't allocate memory for list");
                                kfree(klist_tmr);
                        }
                        klist_tmr->data = jiffies;
                        klist_tmr->thr_num = 1;
                        lock(&my_lock);
                        list_add(&klist_tmr->list, &tmr_list);
                        unlock(&my_lock);
                        printk(KERN_INFO "jiffies added to timer list : %lld\n", klist_tmr->data);
                }                
}
/*
* print_list - printing information from list 
*/
static void print_list(struct list_head *_klist)
{
        struct k_list *_list = NULL;
        printk(KERN_INFO "List data :\n");
        list_for_each_entry(_list, _klist, list) {
                printk(KERN_INFO" jiffies: %lld\n", _list->data);
        }
}
/*
* delete_list - deletes information from list(also delete list)
*/
static void delete_list(struct list_head *_klist)
{
        struct k_list *_list = NULL;
        struct list_head *entry_ptr = NULL, *tmp = NULL;
        list_for_each_safe(entry_ptr, tmp, _klist) {
                _list = list_entry(entry_ptr, struct k_list, list);  
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
        //TODO: add mem error handling
        struct task_struct *kthread_tmr = kmalloc(sizeof(*kthread_tmr), GFP_KERNEL);
        struct task_struct *kthread_wrk = kmalloc(sizeof(*kthread_wrk), GFP_KERNEL);
        if(NULL == kthread_wrk ) {
                printk(KERN_ERR "Can't allocate memory for threads");
                goto errmem;
        }
        if(NULL == kthread_tmr) {
                printk(KERN_ERR "Can't allocate memory for threads");
                goto _errmem;
        }
        INIT_DELAYED_WORK(&my_work, work_handler);
        schedule_delayed_work(&my_work, msecs_to_jiffies(100));
        printk(KERN_INFO "delayed work started!\n");

        mod_timer(&my_timer, jiffies + msecs_to_jiffies(120));
        printk(KERN_INFO "timer started!\n");

	kthread_tmr = kthread_run(thr_tmr_fnc, NULL, "thread%d", 0);	
        kthread_wrk = kthread_run(thr_wrk_fnc, NULL, "thread%d", 1);

	
        flush_scheduled_work();
	return 0;

        errmem:
                kfree(kthread_wrk);
                return 0;
        
        _errmem:
                kfree(kthread_tmr);
                kfree(kthread_wrk);
                return 0;


}
/*
* threads_test_exit - where the battle ends, function that beats up the results, and clears the battlefield(by deallocating memory)
*/
static void __exit threads_test_exit(void)
{
        if(!completion_done(&work_done)) {
                kthread_stop(kthread_wrk);
        }

        if(!completion_done(&timer_done)) {
                kthread_stop(kthread_tmr);
        }

        del_timer_sync(&my_timer);
        cancel_delayed_work(&my_work);
        kfree(kthread_tmr);
        kfree(kthread_wrk);
        delete_list(&tmr_list);
        delete_list(&wrk_list);
        printk(KERN_ALERT "EXIT!\n");
}

module_init(threads_test_init);
module_exit(threads_test_exit);
