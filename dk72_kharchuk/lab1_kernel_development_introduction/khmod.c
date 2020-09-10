/**
 * based on demo code by thodnev
 */

#include <linux/module.h>	 // required by all modules
#include <linux/kernel.h>	 // required for sysinfo
#include <linux/init.h>		 // used by module_init, module_exit macros
#include <linux/jiffies.h>	 // where jiffies and its helpers reside
#include <linux/interrupt.h> // for tasklets

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies, tasklet");
MODULE_AUTHOR("thodnev & morel"); 
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");	//it affects the kernel behavior

static char *user = NULL;
module_param(user, charp, 0);

void tasklet_handler(unsigned long arg)
{
	printk(KERN_INFO "Tasklets's jiffies = %lu\n", jiffies);	
}

DECLARE_TASKLET(kh_tasklet, tasklet_handler, (unsigned long) NULL); 

long start_jiffies;

static int __init firstmod_init(void)
{
	start_jiffies = jiffies;
	if(!user) {
    	printk(KERN_WARNING "Module starts without username");
		printk(KERN_INFO "Hello, $username!\nJiffies = %lu\n", jiffies);	
    } else {
		printk(KERN_INFO "Hello, %s!\n Jiffies = %lu\n", user, jiffies);
	}
	tasklet_schedule(&kh_tasklet);
	return 0;
}
 
static void __exit firstmod_exit(void)
{	
	long delta = jiffies - start_jiffies;
	tasklet_kill(&kh_tasklet);
	printk(KERN_INFO "Long live the Kernel! wrk time - %u sec.\n", jiffies_delta_to_msecs(delta) / 1000);
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);

