#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>	// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>    // where tasklets

MODULE_DESCRIPTION("Module shows tasklet basis for jiffies presentation");
MODULE_AUTHOR("jayawar");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static char *username;

module_param(username, charp, 0);

/* tasklet function */
static void taskl_func(unsigned long param)
{
  	printk(KERN_INFO "Tasklet jiffies: %lu\n", jiffies);

}

/* init tasklet */
DECLARE_TASKLET(jiff_taskl, taskl_func, NULL);

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", username, jiffies);
        tasklet_hi_schedule(&jiff_taskl);
  	printk(KERN_INFO "Tasklet for jiffies printing is running\n");
	return 0;
}

static void __exit firstmod_exit(void)
{
	printk(KERN_INFO "Long live the Kernel!\n");
        /* Stop the runned before tasklet */
        tasklet_kill(&jiff_taskl);
}

module_init(firstmod_init);
module_exit(firstmod_exit);
