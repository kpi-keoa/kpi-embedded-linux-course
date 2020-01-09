/**
 * based on https://bit.ly/2kLBtD9 by thodnev
 * Author: @MaksGolub

 */ 
#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>    //used by tasklet targets
#include "firstmod.h"

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies, tasklet");
MODULE_AUTHOR("MaksHolub");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static char *username = "$username";
module_param(username, charp, S_IRUGO);

static void tasklet_handler(unsigned long jiffy_value);
DECLARE_TASKLET(simple_tasklet, tasklet_handler, 0);

static void tasklet_handler(unsigned long jiffy_value)
{
	printk(KERN_INFO "Run simple tasklet!\n");
	printk(KERN_INFO "Value of Jiffies from tasklet = %lu\n", jiffies);

}

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", username, jiffies);
	tasklet_schedule(&simple_tasklet);
	return 0;
}
 
static void __exit firstmod_exit(void)
{
	tasklet_kill(&simple_tasklet);	
	printk(KERN_INFO "Long live the Kernel!\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);
