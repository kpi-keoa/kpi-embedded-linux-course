#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("rtchoke");
MODULE_VERSION("0.2");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static char *username = "NONE";
module_param(username, charp, 0000);
MODULE_PARM_DESC(username, "Name of user who's using this module");
static void tasklet_handler(unsigned long _unused);
DECLARE_TASKLET(tasklet, tasklet_handler, 0);
static void tasklet_handler(unsigned long _unused)
{
        printk("tasklet jiffies = %lu .\n", jiffies);
}

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", username, jiffies);
        tasklet_schedule(&tasklet);
	return 0;
}
 
static void __exit firstmod_exit(void)
{
        tasklet_kill(&tasklet);
	printk(KERN_INFO "Long live the Kernel!\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);
