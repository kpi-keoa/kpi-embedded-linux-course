#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside
#include <linux/moduleparam.h>    //processing of module input parameters is provided by macros

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("Roman Zherebchenko");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static char *username = NULL;
module_param(username, charp, 0);
static long start_time_jiff;
MODULE_PARM_DESC(username, "User Name");

static int __init testmod_init(void)
{
        start_time_jiff = jiffies;
        if (NULL == username) {
            printk(KERN_WARNING "Name isn't defined!\n");
            username = "$username";
        } else {
            printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", username, jiffies);
        }
        return 0;
}

static void __exit testmod_exit(void)
{
        printk(KERN_INFO "Long live the Kernel!\njiffies = %lu\nTime from init to exit: %u sec\n", jiffies, jiffies_delta_to_msecs(jiffies - start_time_jiff) / 1000);
}

module_init(testmod_init);
module_exit(testmod_exit);

