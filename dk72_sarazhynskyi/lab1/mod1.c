#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("Sarazhynskyi Valentyn");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static long count_start;
static char *usrname = NULL;

module_param(usrname, charp, 0);
MODULE_PARM_DESC(usrname, "user name");

static int __init mod1_init(void)
{
        count_start = jiffies;
        char str[] = "$usrname";
        if (!usrname) {
                printk(KERN_WARNING "username wasn`t passed as a parameter\n");
                usrname = str;
        }

        printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", usrname, jiffies);

        return 0;
}

static void __exit mod1_exit(void)
{
        long delta = jiffies - count_start;
        printk(KERN_INFO "god save the Kernel!\n"
			"work time mnt/mymode.ko : %llu sec\n",
			jiffies_delta_to_msecs(delta) / 1000);
}

module_init(mod1_init);
module_exit(mod1_exit);
