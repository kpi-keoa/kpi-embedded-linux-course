#include <linux/module.h> // required by all modules
#include <linux/kernel.h> // required for sysinfo
#include <linux/init.h> // used by module_init, module_exit macros
#include <linux/jiffies.h> // where jiffies and its helpers reside

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("InterImpv");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL v2");

/* module params */
static char *username = NULL;
module_param(username, charp, 0000);
MODULE_PARM_DESC(username, "Username");

/* module vars */
static unsigned long stime;
static unsigned long long sjiff_64;

/* stolen from kernel/net/ipv4/devinet.c */
#define INITIAL_JIFFIES ((unsigned long)(unsigned int)(-300 * HZ))

static inline u32 cstamp_delta(unsigned long cstamp)
{
	return (cstamp - INITIAL_JIFFIES) / HZ;
}

static int __init firstmod_init(void)
{
	stime = jiffies;
	sjiff_64 = get_jiffies_64();

	if (NULL != username) {
		printk(KERN_INFO "Hello, %s!\n", username);
		printk(KERN_WARNING "%s launched me\n", username);
	} else {
		printk(KERN_INFO "Hello, unknown $username!\n");
		printk(KERN_WARNING "Unknown launched me\n");
	}
	printk(KERN_INFO "Time is %us\n", cstamp_delta(jiffies));

	return 0;
}

static void __exit firstmod_exit(void)
{
	printk(KERN_INFO "Shutting down at jiffies=\t0x%16llx\n", sjiff_64);
	printk(KERN_INFO "Worked for %us\n",
	       jiffies_delta_to_msecs(jiffies - stime) / HZ);
}

module_init(firstmod_init);
module_exit(firstmod_exit);
