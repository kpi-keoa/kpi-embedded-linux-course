#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>	// library, that supports tasklets

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies, tasklet");
MODULE_AUTHOR("bgdpn");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static char *entered_string = "example_of_a_string";

module_param(entered_string, charp, 0);	// charp -> character pointer
MODULE_PARM_DESC(entered_string, "A character string");

static void func(unsigned long tasklet_arg);

// tasklet_name - name of the tasklet
// func - function, that is being executed as a part of the tasklet
// 0 - Data to be passed to the function “func”

DECLARE_TASKLET(tasklet_name, func, 0); 

static void func(unsigned long tasklet_arg)
{
	printk(KERN_INFO "Tasklet is running, jiffie count is: %lu\n", jiffies);
}

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello, %s!\njiffies= %lu\n", entered_string, jiffies);
	func(jiffies);
	return 0;
}
 
static void __exit firstmod_exit(void)
{
	printk(KERN_INFO "Long live the Kernel!\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);
<<<<<<< HEAD

=======
<<<<<<< HEAD

=======
>>>>>>> e2df71a... lab2_simplest_kernel_modules
>>>>>>> df45dd5f5991ab2adcb7d02f42879ac941f6d6d5
