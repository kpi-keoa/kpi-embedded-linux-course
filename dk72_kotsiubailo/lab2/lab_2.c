#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/jiffies.h> 
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/mm.h>

MODULE_DESCRIPTION("timers");
MODULE_AUTHOR("Anton Kotsiubailo");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL"); 

static int cnt = 0;
module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "number of cycles");
int delay = 0;
module_param(delay, int, 0);
MODULE_PARM_DESC(delay, "delay between two timer's works(in miliseconds)");


static typeof(jiffies) *array = NULL;
struct timer_list my_timer;
static int cnt_t = 0;
static int i = 0;

void tasklet_funk(unsigned long arg)
{
	printk(KERN_INFO "Tasklet jiffies is %lu\n", jiffies);
}

DECLARE_TASKLET(tasklet, tasklet_funk, 0);

void array_foo(struct timer_list *data)          
{

	if(cnt_t < cnt) {
		array[cnt_t] = jiffies;
		cnt_t++;
		mod_timer(&my_timer, jiffies + msecs_to_jiffies(delay)); // code was partially taken from Alexander Shlikta work
	}

}

static int __init lab_2_init(void)
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

	tasklet_schedule(&tasklet);
	tasklet_kill(&tasklet);
	
	timer_setup(&my_timer, array_foo, 0);
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(delay));

final:
	return status;
}

static void __exit lab_2_exit(void)
{
	del_timer(&my_timer);

	printk(KERN_INFO "Exit jiffies is %lu\n", jiffies);

	if(cnt_t != cnt)
		printk(KERN_WARNING "Array isn't full\n");

	for (i = 0; i < cnt_t; ++i)
		printk(KERN_INFO "Array[%d] = %lu\n", i, array[i]);
	
	kfree((const void *)array);

	printk(KERN_INFO "Goodbye:)\n");
}

module_init(lab_2_init);
module_exit(lab_2_exit);
