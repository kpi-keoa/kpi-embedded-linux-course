/*
	based on
	https://github.com/kpi-keoa/kpi-embedded-linux-course/blob/master/demo/lab1/firstmod.c
	by thodnev
*/

#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/jiffies.h>	// where jiffies and its helpers reside
#include <linux/interrupt.h> // required by tasklets

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("bramory"); 
MODULE_VERSION("0.1");
MODULE_LICENSE("DualMIT/GPL");	//LICENSE affects the kernel behavior

static char *user = "$username"; //for type "user=<name>"" in the terminal 
module_param(user, charp, S_IRUGO);

void tasklet_handler(unsigned long arg)
{
	printk(KERN_INFO "Tasklets's jiffies = %lu\n", jiffies);	
}

//This structure can be builded by the next MACROS:
//DECLARE_TASKLET(my_tasklet, tasklet_handler, NULL); 
struct tasklet_struct my_tasklet = {
	NULL, 0, ATOMIC_INIT(0), tasklet_handler, (unsigned long) NULL
};

static int __init firstmod_init(void)
{
	printk(KERN_INFO "Hello, %s!\nOur jiffies = %lu\n", user, jiffies);
	tasklet_schedule(&my_tasklet); //waiting for run
	return 0;
}

 
static void __exit firstmod_exit(void)
{
	tasklet_kill(&my_tasklet);
	printk(KERN_INFO "Long live the Kernel! =)\n");
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);
