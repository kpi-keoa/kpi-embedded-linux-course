#include <linux/module.h>		// required by all modules
#include <linux/kernel.h>		// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>		// where jiffies and its helpers reside
#include <linux/moduleparam.h>		// required for using module_param functions
#include <linux/interrupt.h>		// required for using tasklets

MODULE_DESCRIPTION("Simple module demo: init, exit, printk, jiffies, tasklet");
MODULE_AUTHOR("vsilchuk; thodnev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");	

static char *arg_username = "dflt_username";

module_param(arg_username, charp, 0);
MODULE_PARM_DESC(arg_username, "Username value, passed as command line arg.");

void jffs_tasklet_func(unsigned long _unused_data)
{
	printk(KERN_INFO "Tasklet jiffies value = %lu\n", jiffies);
}

DECLARE_TASKLET(jffs_tasklet, jffs_tasklet_func, 0);

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello, %s!\njiffies value = %lu\n", arg_username, jiffies);
	
	tasklet_schedule(&jffs_tasklet);

	return 0;
}
 
static void __exit firstmod_exit(void)
{
	tasklet_kill(&jffs_tasklet);
	printk(KERN_INFO "Long live the Kernel!\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);

