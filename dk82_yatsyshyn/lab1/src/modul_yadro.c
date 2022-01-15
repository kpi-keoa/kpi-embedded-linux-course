#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>   // where jiffies and its helpers reside
#include <linux/time.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("Nazar Yatsishin");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static char *username = NULL;

module_param(username, charp, 0000);
MODULE_PARM_DESC(username, "A character string. Saves username");

static unsigned long jiffies_init;
static struct timespec64 time_jiff;


static int __init mymodule_init(void)
{
	jiffies_init = jiffies;
	if (NULL == username) {
		printk(KERN_WARNING "WARNIGN: Name not set\n");
		username = "$username";
	}
	printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", username, jiffies);
	return 0;
}

static void __exit mymodule_exit(void)
{
	jiffies_to_timespec64(jiffies - jiffies_init, &time_jiff);
	printk(KERN_INFO "Long live the Kernel!\nsecs= %llu\njiffies= %lu\n",
			time_jiff.tv_sec, jiffies);
}

module_init(mymodule_init);
module_exit(mymodule_exit);

