#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("thodnev & andr-17");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static char *hum = NULL;
static long start_jiffies = 0;

module_param(hum, charp, 0);

static int __init firstmod_init(void)
{
    start_jiffies = jiffies;

    if (hum == NULL)
    {
    		printk(KERN_WARNING "Module starts without username", hum, jiffies);
		hum = "$username";
    }

	printk(KERN_INFO "Hello, %s!\n Jiffies = %lu\n", hum, jiffies);
	return 0;
}

static void __exit firstmod_exit(void)
{
	long delta = jiffies - start_jiffies;
	printk(KERN_INFO "Long live the Kernel! Work time - %u sec.\n", jiffies_delta_to_msecs(delta) / 1000);
}

module_init(firstmod_init);
module_exit(firstmod_exit);
