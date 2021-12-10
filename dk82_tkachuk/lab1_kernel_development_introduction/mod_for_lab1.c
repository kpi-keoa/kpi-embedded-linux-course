#include <linux/module.h>	 // required by all modules
#include <linux/kernel.h>	 // required for sysinfo
#include <linux/init.h>		 // used by module_init, module_exit macros
#include <linux/jiffies.h>	 // where jiffies and its helpers reside

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("thodnev & YuriiTk2486");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");

static char *user_name = NULL; //
module_param(user_name, charp, 0); // carry out param.

static long start_time_jiffies = 0; // start-time

static int __init firstmod_init(void) //call-back func. which call when module init.
{
        start_time_jiffies = jiffies;	 // set real_time
        printk("Module: init module... t:%lu\n", start_time_jiffies);

        if(user_name == NULL) {
                printk(KERN_WARNING "Username not set!", user_name, jiffies);
                user_name = "$username";
        }
        printk(KERN_INFO "Hello, %s!\n Jiffies = %lu\n", user_name, jiffies);
        return 0;
}

static void __exit firstmod_exit(void)
{
        long delta = jiffies - start_time_jiffies;
        printk(KERN_INFO "Long live the Kernel! Work time - %u sec.\n", jiffies_delta_to_msecs(delta) / 1000);
}

module_init(firstmod_init);
module_exit(firstmod_exit);
