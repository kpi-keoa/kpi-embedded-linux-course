/* Based on https://bit.ly/2kLBtD9 made by thodnev
 * This code was implemented by:  
 *	-Olexander Ostrianko
 *	-Maxim Salim
 *	-Dana Dovzhenko
 * Task with kernel tasklet was acomplished by Olexander Ostrianko 
 * and Dana Dovzhenko.
 * Parameter transferring to the module was acomplished by Maxim Salim.
 */

#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("alex");
MODULE_VERSION("0.2");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static char *username = "NONAME";
MODULE_PARM_DESC(username, "String reserved for username");

module_param(username, charp, 0000);

void tasklet_function(unsigned long data)
{
	printk(KERN_INFO "Tasklet jiffies = %lu!\n", jiffies);
}

DECLARE_TASKLET(test_tasklet, &tasklet_function, (unsigned long)NULL);

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", username, jiffies);
 	tasklet_schedule(&test_tasklet);
	return 0;
}
 
static void __exit firstmod_exit(void)
{
	tasklet_kill(&test_tasklet);
	printk(KERN_INFO "Long live the Kernel!\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);

