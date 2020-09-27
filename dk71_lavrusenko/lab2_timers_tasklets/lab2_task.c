#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>    // require to user terminal params
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/interrupt.h> // require tasklets
#include <linux/timer.h>

MODULE_DESCRIPTION("Task for lab_2 ");
MODULE_AUTHOR("oleksandr lavrusenko");
MODULE_VERSION("2.0");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static int i;
static int cnt = 0;
static int tmp_cnt = 0;
static int delay;
static typeof(jiffies) *array;
struct timer_list my_timer;

static void my_tasklet_handler(unsigned long flag);

module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "Number of cycle");
module_param(delay, int, 0);
MODULE_PARM_DESC(delay, "Amount of delay");

DECLARE_TASKLET(my_tasklet, my_tasklet_handler, 0);


static void my_tasklet_handler(unsigned long flag)
{
        printk(KERN_INFO "Tasklet jiffies %lu\n", jiffies);
}

/* Some code parts take from Vadim Kharchuck and Dmitro Esich */
void task_func(struct timer_list *data)
{
	if(tmp_cnt < cnt) {
		array[tmp_cnt] = jiffies;
		tmp_cnt++;
		mod_timer(&my_timer, jiffies + msecs_to_jiffies(delay));
	}
}


static int __init lab2_init(void)
{
        int status = 0;

	if (cnt <= 0) {
		printk(KERN_ERR "Invalid <cnt> <= 0 :(\n");
		status = -EINVAL;
		goto final;
	}

	if (delay < 0) {
		printk(KERN_ERR "Invalid <delay> < 0 :(\n");
		status = -EINVAL;
		goto final;
	}

	if ((array = kzalloc(cnt * sizeof(*array), GFP_KERNEL)) == NULL) {
		printk(KERN_ERR "Couldn't allocate a memory:(\n");
		status = -ENOMEM;
		goto final;
	}

	printk(KERN_INFO "%d msec is %lu jiffies\n",delay, msecs_to_jiffies(delay));
	printk(KERN_INFO "Init jiffies is %lu\n", jiffies);

	tasklet_schedule(&my_tasklet);


	timer_setup(&my_timer, task_func, 0);
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(delay));

final:
	return status;
}


static void __exit lab2_exit(void)
{
	del_timer(&my_timer);
        tasklet_kill(&my_tasklet);

	printk(KERN_INFO "Exit jiffies is %lu\n", jiffies);

	if(tmp_cnt != cnt)
		printk(KERN_WARNING "Array isn't full\n");

	for (i = 0; i < tmp_cnt; ++i)
		printk(KERN_INFO "Array[%d] = %lu\n", i, array[i]);

	kfree((const void *)array);

	printk(KERN_INFO "Ave Kernel!\n");
}


module_init(lab2_init);
module_exit(lab2_exit);
