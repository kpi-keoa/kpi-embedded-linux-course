// Originaly made by thodnev
// Modified by CYB3RSP1D3R

#include <linux/module.h>	// required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>	// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h>


MODULE_DESCRIPTION("First module");
MODULE_AUTHOR("CYB3RSP1D3R");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL"); // this affects the kernel behavior

static char *name = "$username"; // variable for the name with default value
module_param(name, charp, 0);
MODULE_PARM_DESC(name, "Variable for passing the name via command line");

static void print_jiffies(unsigned long data);

DECLARE_TASKLET(jiffies_tasklet, print_jiffies, 0); // giving arguement is zero due to its needless

static void print_jiffies(unsigned long data)
{
	printk(KERN_INFO "jiffies using tasklet = %lu\n", jiffies);
}

static int __init init_mod(void)
{
	tasklet_hi_schedule(&jiffies_tasklet);

	printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", name, jiffies);
	
	return 0;
}
 
static void __exit cleanup_mod(void)
{
	tasklet_kill(&jiffies_tasklet);
	
	printk(KERN_INFO "Wanna break your Linux again, %s?\n", name);
	printk(KERN_WARNING "Bye-bye!\n");
}
 
module_init(init_mod);
module_exit(cleanup_mod);
