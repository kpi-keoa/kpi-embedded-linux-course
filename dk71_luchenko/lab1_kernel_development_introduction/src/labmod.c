
#include <linux/module.h>	 // required by all modules
#include <linux/kernel.h>	 // required for sysinfo
#include <linux/init.h>		 // used by module_init, module_exit macros
#include <linux/jiffies.h>	 // where jiffies and its helpers reside

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies, tasklet");
MODULE_AUTHOR("thodnev & luchik"); 
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");	//it affects the kernel behavior

static char *username = NULL;

module_param(username, charp, 0);
MODULE_PARM_DESC(username, "User name");

static long start_jiffies = 0;

static int __init firstmod_init(void)
{
	start_jiffies = jiffies;	

	if(username == NULL) {
    	printk(KERN_WARNING "User name is not defined", username, jiffies);
		username = "$username";	
   	} 

	printk(KERN_INFO "Hello, %s!\n Jiffies = %lu\n", username, jiffies);
	return 0;	
}
 
static void __exit firstmod_exit(void)
{	
	printk(KERN_INFO "Long live the Kernel! wrk time - %u sec.\n", jiffies_delta_to_msecs(jiffies - start_jiffies) / 1000);
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);
