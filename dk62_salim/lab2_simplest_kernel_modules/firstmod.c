/* Based on https://bit.ly/2kLBtD9 made by thodnev
 * This code was implemented by:  
 *	-Olexander Ostrianko
 *	-Maxim Salim
 *	-Dana Dovzhenko
 * Task with kernel tasklet was acomplished by Olexander Ostrianko and Dana Dovzhenko.
 * Parameter transferring to the module was acomplished by Maxim Salim.
 */

#include <linux/module.h>	// required by all modules
#include <linux/moduleparam.h>	// arguments for module
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>	// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>	// used by kernel tasklets 

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("maksimo0");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");// this affects the kernel behavior

static char *received_string = "";

module_param(received_string, charp, 0000);
MODULE_PARM_DESC(received_string, "A character string");

void tasklet_func(unsigned long arg)
{
	printk(KERN_INFO "Jiffies received from tasklet = %lu!\n", jiffies);	
}

DECLARE_TASKLET(task, &tasklet_func, (unsigned long) NULL);

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello, %s!\nJiffies received from init = %lu\n", 
	received_string, jiffies);
	tasklet_hi_schedule(&task);
	return 0;
}
 
static void __exit firstmod_exit(void)
{
	tasklet_kill(&task);
	printk(KERN_INFO "Long live the Kernel!\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);

