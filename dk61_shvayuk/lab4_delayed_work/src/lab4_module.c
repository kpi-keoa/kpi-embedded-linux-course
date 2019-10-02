
#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/kthread.h>  	// for threads
#include <linux/timer.h>
#include <linux/slab.h>		// for kmalloc()
#include <linux/list.h>		// for linked list
#include <asm/atomic.h>		// for atomic "compare-exchange" operation
#include <linux/types.h>	// for atomic_t type declaration
#include <linux/completion.h>   // for using completions
#include <linux/workqueue.h>    // for workqueues

#include <linux/sched/task.h>   // for get_task_struct()

// #define DEBUG_MODE 

#ifdef DEBUG_MODE
#define DEBUG_LOG(message)         \
do {                               \
        printk(KERN_INFO message); \
} while(0)

#define DEBUG_LOG_VAR(message,var_val)      \
do {                                        \
        printk(KERN_INFO message, var_val); \
} while(0)
#else
#define DEBUG_LOG(message)
#define DEBUG_LOG_VAR(message,var_val)
#endif

#define THREADS_RETVAL		(0)
#define TIMER_PERIOD_JIFFIES    (17)
#define COMPARE_DIVIDER_VAL     (11)

MODULE_DESCRIPTION("Module works with Linux kernel timers and workqueues");
MODULE_AUTHOR("max_shvayuk");
MODULE_VERSION("0.228");
MODULE_LICENSE("Dual MIT/GPL");

void wq_callback(struct work_struct *work);
void timer_callback (struct timer_list *data);
int thread_workqueue_handler(void *unused);
int thread_timer_handler(void *unused);

struct task_struct *thread_timer;
struct task_struct *thread_workqueue;

LIST_HEAD(list_timer);
LIST_HEAD(list_workqueue);

struct timer_list *timer_ptr = NULL;
DECLARE_COMPLETION(timer_comp);

struct workqueue_struct *wq_delay_queue = NULL;
DECLARE_DELAYED_WORK(wq_delay_work, wq_callback);

struct list_node
{
	struct list_head next;
	unsigned long data;
};

int thread_timer_handler(void *data)
{
        DEBUG_LOG("Hello from timer thread\n");
        
        /* Creating and configuring the timer */
        timer_ptr = kmalloc(sizeof(*timer_ptr), GFP_KERNEL);
        if (NULL == timer_ptr) {
                printk(KERN_ERR "Can't allocate memory for timer in "
                       "timer_thread\n");
                goto THREAD_END_LABEL;
	}
        timer_setup(timer_ptr, timer_callback, 0);
        mod_timer(timer_ptr, jiffies + TIMER_PERIOD_JIFFIES);

        THREAD_END_LABEL:
        /**
         * The thread waits until the timer send complete "notification" 
         * and deletes the timer 
         */
        wait_for_completion(&timer_comp);
        del_timer_sync(timer_ptr);
        kfree(timer_ptr);
        DEBUG_LOG("Timer thread is going to be stopped\n");
        do_exit(THREADS_RETVAL);
}

void timer_callback(struct timer_list *data)
{
       if (0 == (jiffies % COMPARE_DIVIDER_VAL)) {
                /* Timer send the completion "notification" to the timer's thread */
                complete_all(&timer_comp);
                DEBUG_LOG("Timer stopped the timer_thread just now\n");
        } else {
                struct list_node *node_ptr = kmalloc(sizeof(*node_ptr), 
                                                     GFP_ATOMIC);
                if (NULL == node_ptr) {
                        printk(KERN_ERR "Can't allocate memory for the list "
                               "node in timer\n");
                        complete_all(&timer_comp);
                        return;
                }
                node_ptr->data = jiffies;
                list_add(&(node_ptr->next), &list_timer);
                DEBUG_LOG_VAR("timer: jiffies = %lu\n", jiffies);
                mod_timer(timer_ptr, jiffies + TIMER_PERIOD_JIFFIES);
       }
}

int thread_workqueue_handler(void *data)
{
	DEBUG_LOG("Hello from wq thread\n");

        wq_delay_queue = create_singlethread_workqueue("delayed queue");
        if (NULL == wq_delay_queue) {
                goto TIMER_THREAD_END_LABEL;
        }
        queue_delayed_work(wq_delay_queue, &wq_delay_work, TIMER_PERIOD_JIFFIES);
	
        TIMER_THREAD_END_LABEL:
        while (1) {
		if (kthread_should_stop()) {
                        cancel_delayed_work_sync(&wq_delay_work);
                        destroy_workqueue(wq_delay_queue);
			DEBUG_LOG("wq thread is going to be stopped\n");
                        do_exit(THREADS_RETVAL);
                }
		schedule();
	}
}

/* 
 * This is a 100% copy of to_kthread() func from the kernel/kthread.c file 
 * In kernel/kthread.c file it is decladed as static - so I can't use it in 
 * my file. The simplest solution is Ctrl-C, Ctrl-V it here.
 */
static inline struct kthread *to_kthread(struct task_struct *k)
{
	WARN_ON(!(k->flags & PF_KTHREAD));
	return (__force void *)k->set_child_tid;
}

/* 
 * This is a 100% copy of struct kthread struct from the kernel/kthread.c file 
 * This struct is declared only in .c file - so I can't use it in my file. 
 * The simplest solution is Ctrl-C, Ctrl-V it here.
 */
struct kthread {
	unsigned long flags;
	unsigned int cpu;
	void *data;
	struct completion parked;
	struct completion exited;
#ifdef CONFIG_BLK_CGROUP
	struct cgroup_subsys_state *blkcg_css;
#endif
};

void wq_callback(struct work_struct *work)
{
        if (0 == (jiffies % COMPARE_DIVIDER_VAL)) {
                WQ_THREAD_END_LABEL:
                DEBUG_LOG("wq stopped the wq_thread just now\n");
                /** 
                 * The next several lines are similar to kthread_stop(), 
                 * but my code doesn't wait for the thread is finished
                 */
                #define KTHREAD_SHOULD_STOP (1)
                struct kthread *kthread_;
                get_task_struct(thread_workqueue);
                kthread_ = to_kthread(thread_workqueue);
                set_bit(KTHREAD_SHOULD_STOP, &kthread_->flags);
                wake_up_process(thread_workqueue);
                #undef KTHREAD_SHOULD_STOP
               schedule();
        } else {
                struct list_node *node_ptr = kmalloc(sizeof(*node_ptr), 
                                                     GFP_KERNEL);
                if (NULL == node_ptr) {
                        printk(KERN_ERR "Can't allocate memory "
                               "for the list node in wq\n");
                        goto WQ_THREAD_END_LABEL;
                        return;
                }
                node_ptr->data = jiffies;
                list_add(&(node_ptr->next), &list_workqueue);
                DEBUG_LOG_VAR("wq: jiffies = %lu\n", jiffies);
                queue_delayed_work(wq_delay_queue, 
                                   &wq_delay_work, 
                                   TIMER_PERIOD_JIFFIES);
       }
}

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello from module");
	
	/* Create threads */
        thread_timer = kthread_create(thread_timer_handler, 
                                      NULL, 
                                      "timer thread");
        thread_workqueue = kthread_create(thread_workqueue_handler, 
                                          NULL, 
                                          "timer thread");

	/* Run threads */
        wake_up_process(thread_timer);
        wake_up_process(thread_workqueue);
        
	DEBUG_LOG("Threads started\n");
	
	return 0;
}
 
static void __exit firstmod_exit(void)
{
        /* Stop both timer and wq threads */
        DEBUG_LOG("try to exit\n");
        if (NULL != thread_timer) {
                if(!completion_done(&timer_comp)) {
                        complete(&timer_comp);
                        DEBUG_LOG("Main thread sent the completion "
                               "to stop the timer_thread\n");
                }
        }
        kthread_stop(thread_workqueue);

 	/* Print and delete all nodes of the list_timer */
	struct list_head *it;
	list_for_each(it, &list_timer) {
		unsigned long jiffie_val = (container_of(it, struct list_node, next))->data;
		printk(KERN_INFO "Timer list val is %lu\n", jiffie_val);
	}
	struct list_head *tmp;
	list_for_each_safe(it, tmp, &list_timer) {
		list_del(it);
	}
	DEBUG_LOG("Timer's list deleted\n");

        /* Print and delete all nodes of the wq_timer */
	list_for_each(it, &list_workqueue) {
		unsigned long jiffie_val = (container_of(it, struct list_node, next))->data;
		printk(KERN_INFO "wq list val is %lu\n", jiffie_val);
	}
	list_for_each_safe(it, tmp, &list_workqueue) {
		list_del(it);
	}
        DEBUG_LOG("Workqueue's list deleted\n");

	printk(KERN_INFO "Long live the Kernel!\n");
}

 
module_init(firstmod_init);
module_exit(firstmod_exit);
