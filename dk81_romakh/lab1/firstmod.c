#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>   // where jiffies and its helpers reside
#include <linux/time.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("Vitalii Romakh");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");              // this affects the kernel behavior

static char *username = NULL;

module_param(username, charp, 0000);
MODULE_PARM_DESC(username, "A character string");

/*
 * module_param(foo, int, 0000)
 * The first param is the parameters name
 * The second param is it's data type
 * The final argument is the permissions bits,
 * for exposing parameters in sysfs (if non-zero) at a later stage.
 */

static unsigned long jiffies_init_time;
static struct timespec64 jiffies_value; // 64-bit signed int to store the seconds


static int __init firstmod_init(void)
{
         jiffies_init_time = jiffies;
         if (username == NULL) {
                 username = "$username";
         }
         printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", username, jiffies_init_time);
         return 0;
}

static void __exit firstmod_exit(void)
{
         jiffies_to_timespec64(jiffies - jiffies_init_time, &jiffies_value);
         printk(KERN_INFO "Long live the Kernel!\nTime from init to exit = %usec\n",
                        jiffies_value.tv_sec);
}

module_init(firstmod_init);
module_exit(firstmod_exit);
