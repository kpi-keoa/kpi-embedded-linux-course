#include <linux/module.h> // required by all modules
#include <linux/kernel.h> // required for sysinfo
#include <linux/init.h> // used by module_init, module_exit macros
#include <linux/jiffies.h> // where jiffies and its helpers reside
#include <linux/moduleparam.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("thodnev feat. AlexShlikhta");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL"); // this affects the kernel behavior

static char *username = NULL;

long start_time;

module_param(username, charp, 0);
MODULE_PARM_DESC(username, "Name of user");


static int __init lab_1_init(void)
{
	start_time = jiffies;

	if (!username) {
		printk(KERN_WARNING "<username> wasn't defined:(\n");
		printk(KERN_INFO "Hello, $username!\njiffies = %lu\n", jiffies);

	} else {
		printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", username, 
			jiffies);
	}

	
	return 0;
}

static void __exit lab_1_exit(void)
{
	printk(KERN_INFO "Long live the Kernel!\nWorking time = %u sec\n", 
		jiffies_delta_to_msecs(jiffies - start_time) / 1000);
}

module_init(lab_1_init);
module_exit(lab_1_exit);
