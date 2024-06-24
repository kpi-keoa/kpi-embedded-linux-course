#include <linux/module.h>	 // required by all modules
#include <linux/kernel.h>	 // required for sysinfo
#include <linux/init.h>		 // used by module_init, module_exit macros
#include <linux/jiffies.h>	 // where jiffies and its helpers reside

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies, tasklet");
MODULE_AUTHOR("thodnev & nysh");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");	//it affects the kernel behavior

static char *user = NULL;
module_param(user, charp, 0);

static long start_time_jiffies = 0;

static int __init labfirst_init(void)
{
	start_time_jiffies = jiffies;
	if (user == NULL) {
	    printk(KERN_WARNING "Module starts without username!", user, jiffies);
	    user = "$username";
	}
	printk(KERN_INFO "Hello, %s!\n Jiffies = %lu\n", user, jiffies);
	return 0;

}

static void __exit labfirst_exit(void)
{
	long delta = jiffies - start_time_jiffies;
	printk(KERN_INFO "Long live the Kernel! wrk time - %u sec.\n", jiffies_delta_to_msecs(delta) / 1000);
}

module_init(labfirst_init);
module_exit(labfirst_exit);
