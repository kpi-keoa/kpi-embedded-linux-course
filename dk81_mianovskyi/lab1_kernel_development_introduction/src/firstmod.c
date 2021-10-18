#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>    // required for working with module parameters
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("trueDKstudent; thodnev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static char *name = NULL;
static long start_time = 0;

module_param(name, charp, 0);
MODULE_PARM_DESC(name, "Username");

static int __init firstmod_init(void)
{
        start_time = jiffies;
        if (name == NULL) {
                printk(KERN_WARNING "Username was not provided\n");
                name = "$username";
        }

        printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", name, jiffies);	

        return 0;
}

static void __exit firstmod_exit(void)
{
        long delta_time = jiffies - start_time;
        printk(KERN_INFO "Long live the Kernel!\nworking time is %u sec\n",
               jiffies_delta_to_msecs(delta_time) / 1000);
}

module_init(firstmod_init);
module_exit(firstmod_exit);
