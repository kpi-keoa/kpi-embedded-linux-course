#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside

MODULE_DESCRIPTION("Basic module demo");
MODULE_AUTHOR("raibu");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static char *username = NULL;
module_param(username, charp, 0000);
MODULE_PARM_DESC(username, "User name");
static typeof(jiffies) stime;

static int __init mod_init(void)
{
    stime = jiffies;
    if (username != NULL) {
        printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", username, jiffies);
    } else {
        printk(KERN_INFO "Hello, $username!\njiffies = %lu\n", jiffies);
        printk(KERN_WARNING "User name is not defined\n");
    }
    return 0;
}

static void __exit mod_exit(void)
{
    printk(KERN_INFO "Long live the Kernel! Worked %u seconds\n",
           jiffies_delta_to_msecs(jiffies - stime) / 1000);
}

module_init(mod_init);
module_exit(mod_exit);
