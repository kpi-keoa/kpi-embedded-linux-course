#include <linux/module.h> // required by all modules
#include <linux/kernel.h> // required for sysinfo
#include <linux/init.h> // used by module_init, module_exit macros
#include <linux/jiffies.h> // where jiffies and its helpers reside
#include <linux/moduleparam.h> //used for taking parameters module parameters

MODULE_DESCRIPTION(
	"Basic module demo");
MODULE_AUTHOR("oldem & thodev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL"); // this affects the kernel behavior

static char *nickname;
static long unsigned int  start_time = 0;

module_param(nickname, charp, 0000);
MODULE_PARM_DESC(nickname, "name of user");

static int __init firstmod_init(void)
{
	start_time = jiffies;
	if (nickname == NULL) {
		printk(KERN_WARNING "Something is incorrect with it\n");
		printk(KERN_INFO "Hello, dear $username!\njiffies = %lu\n", jiffies);
	} else
		printk(KERN_INFO "Hello, dear %s!\njiffies = %lu\n", nickname, jiffies);
	return 0;
}

static void __exit firstmod_exit(void)
{
	printk(KERN_INFO "Long live the Kernel, see you next time!\n");
	printk(KERN_INFO "Worked = %lu secs\n",
	       jiffies_delta_to_msecs(jiffies - start_time)/1000);
}

module_init(firstmod_init);
module_exit(firstmod_exit);
