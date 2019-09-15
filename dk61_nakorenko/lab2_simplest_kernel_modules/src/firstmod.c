#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("thodnev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static char *name = "name";

module_param (name, charp, 0000);

void func(unsigned long arg)
{
	printk(KERN_INFO "Tasklet jiffies = %lu\n", arg);
}

DECLARE_TASKLET(tasklet, func, 0);


static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello, %s\njiffies = %lu\n", name, jiffies);
	
	func(jiffies);
	
	return 0;
}


 
static void __exit firstmod_exit(void)
{
	printk(KERN_INFO "Long live the Kernel!\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);

