#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit s
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>    // used for calling tasklet

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies, tasklet");
MODULE_AUTHOR("thodnev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel s

static char *my_parameter = "";

module_param(my_parameter, charp, 0000);
MODULE_PARM_DESC(my_parameter, "An array of integers");


void jiffies_by_tasklet(unsigned long jif)
{
	printk(KERN_INFO "Tasklet jiffies = %lu!\n", jiffies);
}

DECLARE_TASKLET(tasklet, jiffies_by_tasklet, (unsigned long)NULL);

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", my_parameter, jiffies);
	tasklet_hi_schedule(&tasklet);
	return 0;
}
 
static void __exit firstmod_exit(void)
{	
	tasklet_disable(&tasklet);
	printk(KERN_INFO "Long live the Kernel!\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);
