// based on https://gist.github.com/itrobotics/a680c49422a8ff995125 by pantoniou

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/interrupt.h>

MODULE_DESCRIPTION("Basic module: init, deinit, printk, jiffies, tasklet");
MODULE_AUTHOR("bgdpn");
MODULE_VERSION("0.2");
MODULE_LICENSE("Dual MIT/GPL");

static char *name = "username";

module_param(name, charp, 0);
MODULE_PARM_DESC(name, "Name of user");

// without declaration of this function program will not compile
static void tasklet_func(unsigned long _unused);

DECLARE_TASKLET(tasklet_name, tasklet_func, 0); 

static void tasklet_func(unsigned long _unused)
{
	printk(KERN_INFO "Tasklet is running, jiffies is: %lu\n", jiffies);
}

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", name, jiffies);
	tasklet_schedule(&tasklet_name);
	return 0;
}
 
static void __exit firstmod_exit(void)
{
	tasklet_kill(&tasklet_name);
	printk(KERN_INFO "Long live the Kernel!\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);

