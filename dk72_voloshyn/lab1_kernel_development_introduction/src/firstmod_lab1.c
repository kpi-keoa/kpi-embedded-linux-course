#include <linux/module.h>    	// required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>   	// required for sysinfo
#include <linux/init.h>    	// used by module_init, module_exit macros
#include <linux/jiffies.h>   	// where jiffies and its helpers reside

MODULE_DESCRIPTION("Basic module demo");
MODULE_AUTHOR("Anton Voloshyn(sad)");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static char *username = NULL;
static int time_from_start;

module_param(username, charp, 0);
MODULE_PARM_DESC(username, "user name");

static int __init firstmod_init(void)
{
    	time_from_start = jiffies;

    	if(username == NULL) 	
		printk(KERN_WARNING "User wasn't printed his username \n");
		username = "$username";
	}
	
	printk(KERN_INFO "Hello, %s \njiffies = %d \n", username, jiffies); 	

    	return 0;
}

static void __exit firstmod_exit(void)
{
    	printk(KERN_INFO "Long live the Kernel! \n") 
    	printk(KERN_INFO "Working time is - %u seconds \n",
	 		jiffies_delta_to_msecs(jiffies - time_from_start) / 1000);     
}

module_init(firstmod_init);
module_exit(firstmod_exit);
