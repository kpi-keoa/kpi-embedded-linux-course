#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/slab.h>       // to allocate memory
#include <linux/list.h>       // to work with lists
#include <linux/interrupt.h>  //
#include <linux/kthread.h>
#include <asm/atomic.h>       //fot atomic operetion

MODULE_DESCRIPTION("Basic module iteraor: init, deinit, printk, jiffies");
MODULE_AUTHOR("AMD_Inc.");
MODULE_VERSION("0.0.0.1");
MODULE_LICENSE("Dual MIT/GPL");       // this affects the kernel behavior

static int iterator;
static int kernel_quantity;

module_param(iterator, int, 0);
MODULE_PARM_DESC(iterator, "How much to iterate the variable?");

module_param(kernel_quantity, int, 0);
MODULE_PARM_DESC(kernel_quantity, "How many threads to create?");

struct list_rez {
     int rez;
     struct list_head m_list;
};

LIST_HEAD(out_list);
struct list_rez *out_list_acc;

atomic_t *kernel_s = NULL;
int *iter = NULL;

static void lock(atomic_t *arg)
{
     while (atomic_cmpxchg(arg, 0, 1));
}    

static void unlock(atomic_t *arg)
{   
     atomic_set(arg, 0);
}

int thread_f(void *arg)
{
     int *w_arg = (int*)arg;
     lock(kernel_s);
     for (int i = 0; i < iterator; i++) {
          *w_arg += 1;
          schedule();
     }
     printk(KERN_INFO "In fanction thread_f %d \n", w_arg);
     out_list_acc = kmalloc(sizeof(*out_list_acc), GFP_KERNEL);
     if (!out_list_acc) {
          printk(KERN_ERR "Error in out_list\n");
          kfree(out_list_acc);
          return -ENOMEM;
     }
     
     out_list_acc->rez = *w_arg;
     printk(KERN_INFO "In thread_f - list_add - befo\n");
     list_add(&out_list_acc->m_list, &out_list);
     printk(KERN_INFO "In thread_f - list_add - after\n");

     unlock(kernel_s);
     return 0;
}

static int __init lab3iter_init(void)
{
     printk(KERN_INFO "Start init\n");
     kernel_s = kmalloc(sizeof(*kernel_s), GFP_KERNEL);
     if (!kernel_s) {
          printk(KERN_ERR "Error in kernel_s\n");
          kfree(kernel_s);
          return -ENOMEM;
     }
     
     atomic_set(kernel_s, 0);

     iter = kmalloc(sizeof(*iter), GFP_KERNEL);
     if (!kernel_s) {
          printk(KERN_ERR "Error in iter\n");
          kfree(iter);
          kfree(kernel_s);
          return -ENOMEM;
     }

     struct task_struct **core_thread;
     core_thread = NULL;
     core_thread = kmalloc(sizeof(*core_thread) * kernel_quantity, GFP_KERNEL);
     if (!kernel_s) {
          printk(KERN_ERR "Error in core_thread\n");
          kfree(iter);
          kfree(kernel_s);
          kfree(core_thread);
          return -ENOMEM;
     }
     *iter = 0;
     for (int i = 0; i < kernel_quantity; i++) {
          core_thread[i] = kthread_run(&thread_f, (void *)iter, "thread_%d", i);
     }
     printk(KERN_INFO "Kol iteration %d\n", kernel_quantity);
     printk(KERN_INFO "Kol iteration %d\n", iterator);
     printk(KERN_INFO "Done init\n");
     return 0;
}
 
static void __exit lab3iter_exit(void)
{
     printk(KERN_INFO "Start exit\n");
     if (iter) {
          printk(KERN_INFO "Kol iteration %d\n", *iter);
          kfree(iter);
     }
     struct list_head *pos;
     struct list_head *q;
     printk(KERN_INFO "Start print list\n");
     list_for_each_safe(pos, q, &out_list) {
          out_list_acc = list_entry(pos, struct list_rez, m_list);
          printk(KERN_NOTICE "--%d\n", out_list_acc->rez);
          list_del(pos);
          kfree(out_list_acc);
     }
     kfree(kernel_s);
     printk(KERN_INFO "Done exit\n");
}

module_init(lab3iter_init);
module_exit(lab3iter_exit);




і