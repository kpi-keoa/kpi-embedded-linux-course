#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>    // require to user terminal params
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>    // where jiffies and its helpers reside

MODULE_DESCRIPTION("Task for lab_1 ");
MODULE_AUTHOR("oleksandr lavrusenko");
MODULE_VERSION("1.0");
MODULE_LICENSE("Dual MIT/GPL");    // this affects the kernel behavior

static long start;

static char *mystring = NULL;
module_param(mystring, charp, 0000);
MODULE_PARM_DESC(mystring, "A character string");

static int __init task_func_init(void)
{
        start = jiffies;

        if (mystring != NULL) {
                printk(KERN_WARNING "Username was not entered into the command line\n");
                printk(KERN_INFO "Hello, $username!\n");

        } else {
                printk(KERN_INFO "Hello,  %s!\njiffies = %lu\n", mystring, jiffies);
        }

        return 0;
}

static void __exit task_func_exit(void)
{
        long delta = jiffies - start;
        printk(KERN_INFO "Ave Kernel!\n"
                         "time spent %u sec\n",
               jiffies_delta_to_msecs(delta) / 1000);
}

module_init(task_func_init);
module_exit(task_func_exit);
