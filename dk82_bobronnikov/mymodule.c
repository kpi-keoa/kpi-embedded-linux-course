#include <linux/module.h> // required by all modules
#include <linux/kernel.h> // required for sysinfo
#include <linux/init.h> // used by module_init, module_exit macros
#include <linux/jiffies.h> // where jiffies and its helpers reside
#include <linux/moduleparam.h> //used for taking parameters module parameters

MODULE_DESCRIPTION(
	"Basic module demo: init, deinit, printk, jiffies, moduleparam");
MODULE_AUTHOR("Andrii Bobronnikov");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL"); // this affects the kernel behavior

static char *name;
long unsigned int jiffies_start_time;

module_param(name, charp, 0000);
MODULE_PARM_DESC(name, "A string with entered username");

static int __init firstmod_init(void)
{
	jiffies_start_time = jiffies;
	if (name == NULL) {
		printk(KERN_WARNING "Name is not specified! Using standart:\n");
		printk(KERN_INFO "Hello, $username!\njiffies = %lu\n", jiffies);
	} else
		printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", name, jiffies);
	return 0;
}

static void __exit firstmod_exit(void)
{
	printk(KERN_INFO "Long live the Kernel!\n");
	printk(KERN_INFO "jiffies delta = %lu msecs\n",
	       jiffies_delta_to_msecs(jiffies - jiffies_start_time));
}

module_init(firstmod_init);
module_exit(firstmod_exit);
