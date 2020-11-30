#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>    // require to user terminal params
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/interrupt.h> // require tasklets
#include <linux/timer.h>

MODULE_DESCRIPTION("Lab_2 task : tasklets_timers ");
MODULE_AUTHOR("latyshev yan");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");   

static int i;
static int counter_kk = 0;
static typeof(jiffies) *my_array;
struct timer_list timer;

static void my_tasklet_func(unsigned long flag);
static int cnt = 0;
module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "ammount of cycles that timer works");
static int delay;
module_param(delay, int, 0);
MODULE_PARM_DESC(delay, "a delay of the timer");

DECLARE_TASKLET(tasklet, my_tasklet_func, 0);


static void my_tasklet_func(unsigned long flag)
{
        printk(KERN_INFO "!! Tasklet --> jiffies is %lu\n", jiffies);
}

/* Some code parts taken from Lavrusenko O. */
void task_func(struct timer_list *data)
{
	if(counter_kk < cnt) {
		my_array[counter_kk] = jiffies;
		counter_kk++;
		mod_timer(&timer, jiffies + msecs_to_jiffies(delay));
	}
}


static int __init lab2_main_func_init(void)
{
	int stat = 0;

	if (cnt <= 0) 
	{
		printk(KERN_ERR "!! Invalid value cnt < 0 \n");
		stat = -EINVAL;
		goto final;
	}
	else if (cnt > 0) 
	{
		printk(KERN_ERR "!! Nice --> cnt > 0 \n");
	}


	if (delay <= 0) 
	{
		printk(KERN_ERR "!! Invalid value delay < 0\n");
		stat = -EINVAL;
		goto final;
	}
	else if (delay > 0) 
	{
		printk(KERN_ERR "!! Nice --> delay > 0\n");
	}


	if ((my_array = kzalloc(cnt * sizeof(*my_array), GFP_KERNEL)) == NULL)
	{
		printk(KERN_ERR "!! Couldn't allocate ohhh bad bad bad bad\n");
		stat = -ENOMEM;
		goto final;
	}

	if ((cnt <= 0) || (delay <= 0))  
	{
		printk(KERN_INFO "!! take it once ... my Init --> jiffies is %lu\n", jiffies);
	}


	printk(KERN_INFO "!! %d msec is our delay \n",delay);
	printk(KERN_INFO "!! it is equal to %lu jiffies\n",msecs_to_jiffies(delay));
	printk(KERN_INFO "!! Init --> jiffies is %lu\n", jiffies);

	tasklet_schedule(&tasklet);
	timer_setup(&timer, task_func, 0);
	mod_timer(&timer, jiffies + msecs_to_jiffies(delay));

final:
	return stat;
}


static void __exit lab2_main_func_exit(void)
{
	del_timer(&timer);
        tasklet_kill(&tasklet);

	printk(KERN_INFO "!! Exit value --> jiffies is %lu\n", jiffies);


	if(counter_kk != cnt)
		printk(KERN_WARNING "Array isn't full\n");

	for (i = 0; i < counter_kk; ++i)
		printk(KERN_INFO "!! My Array element %d = %lu\n", i, my_array[i]);

	kfree((const void *)my_array);

	printk(KERN_INFO "GG WP Kernel!\n");
}


module_init(lab2_main_func_init);
module_exit(lab2_main_func_exit);
