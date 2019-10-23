#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>	// used by kmod_init, kmod_exit macros
#include <linux/kthread.h>	// used by kernel threads
#include <linux/list.h>	// used by lists
#include <linux/slab.h>	// used for memory allocating
#include <linux/timer.h>	// used by kernel timer
#include <linux/workqueue.h>	// used by workque

MODULE_DESCRIPTION("lab4");
MODULE_AUTHOR("AMD_Inc.");
MODULE_VERSION("0.0.0.0.1");
MODULE_LICENSE("Dual MIT/GPL");	

int flags[2];

struct struct_list {
     long int ticks;
     struct list_head list;
};

LIST_HEAD(first_list);
LIST_HEAD(second_list);

struct delayed_work my_work;
struct timer_list my_timer;

struct task_struct **threads_crs;

/*void my_timer_func(unsigned long data)*/
void my_timer_func(struct timer_list *data)
{
     printk(KERN_INFO "In fanction my_timer_func \n");
     long int temp_jif = jiffies;
     if ((temp_jif % 11) == 0) {
          flags[0] = false;
     } else {
          struct struct_list *temp_list;
          temp_list = kmalloc(sizeof(*temp_list), GFP_ATOMIC);
          if (temp_list) {
               temp_list->ticks = temp_jif;
               list_add(&(temp_list->list), &first_list);
               mod_timer(&my_timer, jiffies + 17);
          } else {
               printk(KERN_ERR "Memory Allocation Error!\n");
               flags[0] = false;
          }
     }

}

void my_work_func(struct work_struct *data)
{

     printk(KERN_INFO "In fanction my_work_func \n");
     long int temp_jif = jiffies;
     if ((temp_jif % 11) == 0) {
	  flags[1] = false;
     } else {
          struct struct_list *temp_list;
          temp_list = kmalloc(sizeof(*temp_list), GFP_KERNEL);
          if (temp_list) {
               temp_list->ticks = temp_jif;
               list_add(&(temp_list->list), &second_list);
               schedule_delayed_work(&my_work, 17);
          } else {
               printk(KERN_ERR " Memory Allocation Error!\n");
               flags[1] = false;
          }
     }

}

void print_list(struct list_head *data_list)
{

     struct struct_list *temp_list;
     list_for_each_entry(temp_list, data_list, list) {
          printk(KERN_NOTICE " --- %11d\n", temp_list->ticks);
     }
}

void del_list(struct list_head *data_list)
{

     struct struct_list *temp_list;
     struct list_head *pos;
     struct list_head *q;

     list_for_each_safe(pos, q, data_list) {
          temp_list=list_entry(pos, struct struct_list, list);
          list_del(pos);
          kfree(temp_list);
     }

}

int thr_first_func(void *data)
{

     printk(KERN_INFO "In fanction thr_first_func \n");
     while (flags[0]) {
          schedule();
     }
     printk(KERN_NOTICE "First list items (timer)\n");
     print_list(&first_list);
     return 0;

}

int thr_second_func(void *data)
{

     printk(KERN_INFO "In fanction thr_second_func \n");
     while (flags[1]) {
          schedule();
     }
     printk(KERN_NOTICE "Second list items (workqueue)\n");
     print_list(&second_list);
     return 0;

}

static int __init mod_init(void) 
{
     printk(KERN_INFO "Start init\n");
     threads_crs = kmalloc(sizeof(*threads_crs) * 2, GFP_KERNEL);

     if (!threads_crs) {
          printk(KERN_ERR " Memory Allocation Error!\n");
          kfree(threads_crs);
          return -ENOMEM;
     }

     flags[0] = true;
     printk(KERN_INFO "Init %d\n", flags[0]);
     threads_crs[0] = kthread_run(&thr_first_func, NULL, "kthread1");

     flags[1] = true;
     printk(KERN_INFO "kthread_run %d\n", flags[1]);
     threads_crs[1] = kthread_run(&thr_second_func, NULL, "kthread2");

     printk(KERN_INFO "timer setup %d\n", flags[0]);
     timer_setup(&my_timer, &my_timer_func, 0);
     printk(KERN_INFO "mod_timer %d\n", flags[0]);
     mod_timer(&my_timer, jiffies + msecs_to_jiffies(10));

     printk(KERN_INFO "INIT_DELAYED_WORK %d\n", flags[0]);
     INIT_DELAYED_WORK(&my_work, my_work_func);
     schedule_delayed_work(&my_work, msecs_to_jiffies(20));
     printk(KERN_INFO "Done init\n");
     return 0;

}

static void __exit mod_exit(void) 
{
     printk(KERN_INFO "Start exit\n");
     for (int i = 0; i < 2; i++) {
          if (flags[i]) {
               kthread_stop(threads_crs[i]);
               flags[i] = false;
          }
     }

     del_timer_sync(&my_timer);
     flush_delayed_work(&my_work);

     printk(KERN_NOTICE "DELETE list items (timer)\n");
     del_list(&first_list);
     printk(KERN_NOTICE "DELETE list items (work)\n");
     del_list(&second_list);

     kfree(threads_crs);

     printk(KERN_NOTICE "Module unloaded!\n" ); 
}


module_init(mod_init);
module_exit(mod_exit);



