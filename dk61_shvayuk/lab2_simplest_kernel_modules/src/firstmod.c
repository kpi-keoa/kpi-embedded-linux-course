/**
 * Based on the thodnev's code example:
 * https://github.com/kpi-keoa/kpi-embedded-linux-course/blob/master/demo/lab1/firstmod.c
 */
#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/interrupt.h>

MODULE_DESCRIPTION("Basic module demo (extended):"
                   " init, deinit, printk, jiffies, current time");
MODULE_AUTHOR("max_shvayuk");
MODULE_VERSION("0.228");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

void tasklet_jiffies_handler(unsigned long data);

struct timespec human_readable_time = { 0 };
static char *arg_str = "default";
module_param(arg_str, charp, 0);
MODULE_PARM_DESC(arg_str, "A string to be displayed at all module's "
                          "log messages");
DECLARE_TASKLET(tasklet_jiffies_print, tasklet_jiffies_handler, 0);

void tasklet_jiffies_handler(unsigned long data)
{
	getnstimeofday(&human_readable_time);
	
	printk(KERN_INFO "Hello from tasklet, %s!\n"
		   "jiffies = %lu\nseconds = %llu\n", arg_str,
           jiffies, (unsigned long long)human_readable_time.tv_sec);
	
	tasklet_schedule(&tasklet_jiffies_print);
}

static int __init firstmod_init(void)
{
	getnstimeofday(&human_readable_time);

	tasklet_schedule(&tasklet_jiffies_print);
	
	
	printk(KERN_INFO "Hello, %s!\njiffies = %lu\nseconds = %llu\n", arg_str,
           jiffies, (unsigned long long)human_readable_time.tv_sec);
	return 0;
}
 
static void __exit firstmod_exit(void)
{
	tasklet_kill(&tasklet_jiffies_print);
	printk(KERN_INFO "Long live the Kernel!\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);
