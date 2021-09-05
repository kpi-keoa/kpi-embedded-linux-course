#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside


MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("A.Kovalenko");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static char *username = "";

module_param(username, charp, 0000);
MODULE_PARM_DESC(username, "User Name");

unsigned long js, je, dif;

static int __init firstmod_init(void)
{
    if (username=="") {
        printk(KERN_WARNING "username not defined");
    }
    js = jiffies;
    printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", username, js);
	return 0;
}

static void __exit firstmod_exit(void)
{
    je = jiffies;
    printk(KERN_INFO "Goodbye, %s!\njiffies = %lu\n Long live the Kernel!\n", username, je);
    dif = je - js;
    printk(KERN_INFO "Total elapsed time = %lu\n", dif);
    printk(KERN_INFO "Seconds time = %u\n", jiffies_delta_to_msecs(dif)/1000);
}
 
module_init(firstmod_init);
module_exit(firstmod_exit); 
