#include <linux/module.h>   
#include <linux/moduleparam.h>
#include <linux/kernel.h>    
#include <linux/init.h>    
#include <linux/jiffies.h>    

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("Yan Latyshev");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");  


static char *usern = NULL;
module_param(usern, charp, 0);
MODULE_PARM_DESC(usern, "User Name");

static long start_of_jiff;
static int __init firstmod_my(void)
{
        start_of_jiff = jiffies;
        if (usern == NULL) {
                printk(KERN_INFO "Hello, $nouname!\n jiffies = %lu\n", jiffies);
                printk(KERN_WARNING "User name is not defined\n");
        } else if (usern != NULL){
                printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", usern, jiffies);
        }
        return 0;
}

static void __exit secondmod_my(void)
{
        long difference = jiffies - start_of_jiff;
        printk(KERN_INFO "god my Kernel!\n Worked %llu seconds\n",
			jiffies_delta_to_msecs(difference) / 1000);
}

module_init(firstmod_my);
module_exit(secondmod_my);
