#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h> // where jiffies and its helpers reside
#include <linux/kernel.h>  // required for sysinfo
#include <linux/module.h>  // required by all modules

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("Anton Kotsiubailo");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL"); // this affects the kernel behavior

static long start_time = 0;
static char *username = NULL;

module_param(username, charp, 0);
MODULE_PARM_DESC(username, "A user name string");

static int __init lab_1_init(void) {
  start_time = jiffies;

  if (!username) {
    username = "$username";
    printk(KERN_WARNING "username was'nt defined !!!\n");
  }

  printk(KERN_INFO "Hello, %s!\njiffies = %lu\n", username, jiffies);

  return 0;
}

static void __exit lab_1_exit(void) {
  printk(KERN_INFO "Long live the Kernel! Working time = %u sec\n",
         jiffies_delta_to_msecs(jiffies - start_time) / 1000);
}

module_init(lab_1_init);
module_exit(lab_1_exit);
