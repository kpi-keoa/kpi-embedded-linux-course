/**
 * based on
 * https://github.com/kpi-keoa/kpi-embedded-linux-course/
 * blob/master/demo/lab1/firstmod.c
 * by thodnev
 */

#include <linux/module.h>	 // required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>	 // required for sysinfo
#include <linux/init.h>		 // used by module_init, module_exit macros
#include <linux/jiffies.h>	 // where jiffies and its helpers reside


MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("cartman; thodnev"); 
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");	//it affects the kernel behavior

static char *user = NULL;
static int beginning;

module_param(user, charp, 0000);
MODULE_PARAM_DESC(user, "Username");

static int __init firstmod_init(void)
{
	beginning = jiffies;

    if (user!= NULL) {
        printk(KERN_WARNING "Username was not entered into the command line\n");
        printk(KERN_INFO "Hello, $username!\n");

    } else {
        printk(KERN_INFO "Hello,  %s!\njiffies = %lu\n", user, jiffies);
    }
}
 
static void __exit firstmod_exit(void)
{
	printk(KERN_INFO "Long live the Kernel! =)\n");
	printk(KERN_INFO "Working time is - %u seconds \n",
	 		jiffies_delta_to_ms(jiffies - beginning) / 1000);     
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);
