/**
 * based on demo by thodnev & lab_1 code
 */

#include <linux/module.h>	 // required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>	 // required for sysinfo
#include <linux/init.h>		 // used by module_init, module_exit macros
#include <linux/jiffies.h>	 // where jiffies and its helpers reside
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/mm.h>

MODULE_DESCRIPTION("timers,memory");
MODULE_AUTHOR("thodnev & morel"); 
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");	//it affects the kernel behavior

static int cnt = 0;
module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "number of addings");

static int delay = 0;
module_param(delay, int, 0);
MODULE_PARM_DESC(cnt, "period in ms");

static typeof(jiffies) *array;
struct timer_list tim;
static int cnt_t = 0;
static int i = 0;

void tasklet_kh(unsigned long arg)
{
	printk(KERN_INFO "Tasklet jiffies is %lu\n", jiffies);
}

void jif_to_array(struct timer_list *data)
{
	if(cnt_t < cnt) {
		array[cnt_t] = jiffies;
		cnt_t++;
		mod_timer(&tim, jiffies + msecs_to_jiffies(delay));
	}
}


DECLARE_TASKLET(tasklet, tasklet_kh, 0);

static int __init khmod_init(void)
{
	printk(KERN_INFO "Initial Jiffies is %lu\n", jiffies);
	tasklet_schedule(&tasklet);
	
	if ((array = kzalloc(cnt * sizeof(*array), GFP_KERNEL)) == NULL) {
		printk(KERN_ERR "Allocate error\n");
		goto fail;
	}
	
	if (cnt <= 0) {
		printk(KERN_ERR "Wrong cnt\n");
		goto fail;		
	}

	if (delay < 0) {
		printk(KERN_ERR "Wrong delay\n");
		goto fail;		
	}
	
	timer_setup(&tim, jif_to_array, 0);
	mod_timer(&tim, jiffies + msecs_to_jiffies(delay));
	return 0;
	
fail:
	printk(KERN_ERR "Terminated\n");
	return -1;
}
 
static void __exit khmod_exit(void)
{	
	printk(KERN_INFO "Exit jiffies is %lu\n", jiffies);
	
	del_timer(&tim);
	
	if(cnt_t < cnt)
		printk(KERN_WARNING "Not passed\n");
	
	for (i = 0; i < cnt_t; i++) 
		printk(KERN_INFO "Array[%d] = %lu\n", i, array[i]);	
		
	kfree((const void *)array);
}
 
module_init(khmod_init);
module_exit(khmod_exit);

