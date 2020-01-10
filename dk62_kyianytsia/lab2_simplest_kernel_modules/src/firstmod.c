#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("HL36");
MODULE_VERSION("0.1.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static char *username;
MODULE_PARM_DESC(username, "username");
module_param(username, charp, 0000);

static void tasklet_func(unsigned long arg);
DECLARE_TASKLET(print_jiffies, tasklet_func, (unsigned long)NULL);

static int __init firstmod_init(void)
{
	tasklet_schedule(&print_jiffies);
	
	printk(KERN_INFO "Hello, %s!\n jiffies = %lu\n", username, jiffies);
	return 0;
}
 
static void __exit firstmod_exit(void)
{
	printk(KERN_INFO "Long live the Kernel!\n");
	tasklet_kill(&print_jiffies);
}
 
static void tasklet_func(unsigned long arg)
{
	printk(KERN_INFO "jiffies = %lu\n", jiffies);
	
}

module_init(firstmod_init);
module_exit(firstmod_exit);

