#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("sokol");
MODULE_VERSION("0.1.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static char *username = "Sokol";
MODULE_PARM_DESC(username, "username");
module_param(username, charp, 0000);

static void tasklet_func(unsigned long arg);
DECLARE_TASKLET(name_tasklet, tasklet_func, (unsigned long)NULL);

static int __init firstmod_init(void)
{
	tasklet_schedule(&name_tasklet);

	printk(KERN_INFO "$$$ Hello, %s $$$\njiffies = %lu\n", username, jiffies);
	return 0;
}
 
static void __exit firstmod_exit(void)
{
	tasklet_kill(&name_tasklet);
	printk(KERN_INFO "$$$ Long live the Kernel! $$$\n");
}


static void tasklet_func(unsigned long arg)
{
	printk(KERN_INFO "Tasklet: \njiffies = %lu\n", jiffies);
}

 
module_init(firstmod_init);
module_exit(firstmod_exit);

