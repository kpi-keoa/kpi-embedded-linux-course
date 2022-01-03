#include <linux/module.h>	 // required by all modules
#include <linux/kernel.h>	 // required for sysinfo
#include <linux/init.h>		 // used by module_init, module_exit macros
#include <linux/jiffies.h>	 // where jiffies and its helpers reside

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies, tasklet");
MODULE_AUTHOR("thodnev & Your_Sweetheart");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

static char *user_name = NULL; //
module_param(user_name, charp, 0); // carry out param.

static long st_t_jiffies = 0; // start-time

static int __init firstmod_init(void) //call-back func. which call when module init.
{
	st_t_jiffies = jiffies;	 // set real_time
	printk("Module: init module... t:%lu\n", st_t_jiffies);

	if(user_name == NULL) {
    	printk(KERN_WARNING "Module starts without username!!!", user_name, jiffies);
		user_name = "$username";
    }
	printk(KERN_INFO "Hello, %s!\n Jiffies = %lu\n", user_name, jiffies);
	return 0;

}

static void __exit firstmod_exit(void)
{
	long delta = jiffies - st_t_jiffies;
	printk(KERN_INFO "Long live the Kernel! wrk time - %u sec.\n", jiffies_delta_to_msecs(delta) / 1000);
}

module_init(firstmod_init);
module_exit(firstmod_exit);
