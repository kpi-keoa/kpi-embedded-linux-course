#include <linux/module.h>       // required by all modules
#include <linux/kernel.h>       // required for sysinfo
#include <linux/init.h>         // used by module_init, module_exit macros
#include <linux/jiffies.h>      // where jiffies and its helpers reside
#include <linux/interrupt.h>    // tasklets
#include <linux/timer.h>        // timer
#include <linux/slab.h>         // memory allocation

MODULE_DESCRIPTION("Init, deinit, printk, jiffies, tasklet, timer, memory allocation");
MODULE_AUTHOR("YuriiTk2486");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static long cnt = 2;
static unsigned long delay = 200;
static unsigned long *array_of_jiffies;
static unsigned long current_counter;

module_param(cnt, long, 0);
module_param(delay, ulong, 0);

MODULE_PARM_DESC(cnt, "A parameter that determines the number of timer counts");
MODULE_PARM_DESC(delay, "Duration of one timer countdown");

static void tasklet_handler_jiffies(struct tasklet_struct *data);
DECLARE_TASKLET(tasklet_jiffies, tasklet_handler_jiffies);

static void timer_handler(struct timer_list *list);
DEFINE_TIMER(timer, timer_handler);

static int __init mod_for_lab2_init(void)
{
        printk(KERN_INFO "Init: Jiffies = %lu\n", jiffies);
	tasklet_schedule(&tasklet_jiffies);

        current_counter = 0;

	if (0 == cnt) {
		printk(KERN_WARNING "Init: cnt == 0\n");
		return 0;
	}

	array_of_jiffies = kmalloc(sizeof(cnt) * cnt, GFP_KERNEL);

	if (NULL == array_of_jiffies) {
		printk(KERN_ERR "Init: Memory cannot be allocated (array_of_jiffies == NULL)\n");
		return 0;
	}

	if (0 == delay)
		printk(KERN_WARNING "Init: delay = 0\n");

	mod_timer(&timer, jiffies + delay);

	return 0;
}

static void tasklet_handler_jiffies(struct tasklet_struct *data)
{
        printk(KERN_INFO "Tasklet: Jiffies = %lu\n", jiffies);
}

static void timer_handler(struct timer_list *list)
{
	array_of_jiffies[current_counter] = jiffies;
	current_counter++;

	if (current_counter < cnt)
		mod_timer(&timer, jiffies + delay);
}

static void __exit mod_for_lab2_exit(void)
{
	long i;

        printk(KERN_INFO "Exit: Jiffies= %lu\n", jiffies);

	tasklet_kill(&tasklet_jiffies);

	if (timer_pending(&timer))
		printk(KERN_WARNING "Exit: Timer kill\n");

	del_timer(&timer);

        if (NULL != array_of_jiffies) {
                for (i = 0; i < current_counter; i++)
                        printk(KERN_INFO "Exit: array_of_jiffies[%lu] jiffies= %lu\n", i, array_of_jiffies[i]);
        }

	if (NULL != array_of_jiffies)
		kfree(array_of_jiffies);
}

module_init(mod_for_lab2_init);
module_exit(mod_for_lab2_exit);
