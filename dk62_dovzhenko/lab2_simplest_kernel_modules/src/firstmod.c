/*	Based on demo-code made by thodnev.
	
	This code was implemented by:  
		-Olexander Ostrianko
		-Maxim Salim
		-Dana Dovzhenko
	Task with kernel tasklet was acomplished by Olexander Ostrianko and Dana Dovzhenko.
	Parameter transferring to the module was acomplished by Maxim Salim. */


#include <linux/module.h>		// required by all modules
#include <linux/kernel.h>		// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/moduleparam.h>
#include <linux/jiffies.h>		// where jiffies and its helpers reside
#include <linux/interrupt.h>	
#include <linux/time.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("thodnev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior
MODULE_PARM_DESC(username, "user name");

static char *username = "without name";
module_param(username, charp, 0000);

void tasklet_jiffies(unsigned long arg)
{
	printk(KERN_INFO "Jiffies from tasklet = %lu!\n", jiffies);
}

DECLARE_TASKLET(tasklet, tasklet_jiffies, (unsigned long)NULL);

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello, %s!\n jiffies from init = %lu\n", username, jiffies);
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

