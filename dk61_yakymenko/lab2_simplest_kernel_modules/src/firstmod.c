#include <linux/module.h>	// required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("thodnev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static char *name = "oleh";
module_param(name, charp, 0000);
MODULE_PARM_DESC(name, "User name");

static void tasklet_fun(unsigned long flag);
DECLARE_TASKLET(jiffies_tasklet, tasklet_fun, 0);

static void tasklet_fun(unsigned long flag)
{
	printk(KERN_INFO "TASKLET jiffies = %lu\n", jiffies);
}

static int __init firstmod_init(void)
{
	tasklet_schedule(&jiffies_tasklet);

	printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", name, jiffies);
	
	return 0;
}
 
static void __exit firstmod_exit(void)
{
	tasklet_kill(&jiffies_tasklet);
	printk(KERN_INFO "Long live the Kernel!\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);
