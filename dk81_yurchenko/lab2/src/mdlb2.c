#include <linux/module.h>    // required by all modules
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>    // used by module_init, module_exit macros
#include <linux/jiffies.h>   // where jiffies and its helpers reside
#include <linux/slab.h>
#include <linux/interrupt.h>    //tasklets
#include <linux/timer.h>
#include <linux/time.h>

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("Vitalii Yurchenko");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static long cnt = 4;
static unsigned long delay = 100;
static unsigned long *array;
static unsigned long ticks;

module_param(cnt, long, 0000);
module_param(delay, ulong, 0000);
MODULE_PARM_DESC(cnt, "Saves cnt");
MODULE_PARM_DESC(delay, "Saves delay");

static void my_tasklet_handler(struct tasklet_struct *data);
DECLARE_TASKLET(my_tasklet, my_tasklet_handler);

static void timer_fn(struct timer_list *list);
DEFINE_TIMER(my_timer, timer_fn);

static int __init mymodule_init(void)
{
	ticks = 0;

	printk(KERN_INFO "mdlb2: module: jiffies = %lu\n", jiffies);
	tasklet_schedule(&my_tasklet);

	if (0 == cnt) {
		pr_warn("mdlb2: cnt = 0\n");
		return 0;
	}

	array = kmalloc(sizeof(cnt) * cnt, GFP_KERNEL);

	if (NULL == array) {
		pr_err("mdlb2: array = NULL\n");
		return 0;
	}

	if (0 == delay)
		pr_warn("mdlb2: delay = 0\n");

	mod_timer(&my_timer, jiffies + delay);

	return 0;
}

static void my_tasklet_handler(struct tasklet_struct *data)
{
	pr_info("mdlb2: tasklet: jiffies = %lu\n", jiffies);
}

static void timer_fn(struct timer_list *list)
{
	array[ticks] = jiffies;
	ticks++;

	if (ticks < cnt)
		mod_timer(&my_timer, jiffies + delay);
}


static void __exit mymodule_exit(void)
{
	long i;

	printk(KERN_INFO "mdlb2: module: jiffies= %lu\n", jiffies);

	tasklet_kill(&my_tasklet);

	if (timer_pending(&my_timer))
		pr_warn("mdlb2: timer kill\n");

	del_timer(&my_timer);

	for (i = 0; i < ticks; i++)
		pr_info("mdlb2: timer: array[%lu] jiffies= %lu\n", i, array[i]);

	if (NULL != array)
		kfree(array);
}

module_init(mymodule_init);
module_exit(mymodule_exit);

