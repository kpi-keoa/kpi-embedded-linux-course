#include <linux/module.h>       // required by all modules
#include <linux/kernel.h>       // required for sysinfo
#include <linux/init.h>         // used by module_init, module_exit macros
#include <linux/jiffies.h>      // where jiffies and its helpers reside
#include <linux/slab.h>         // memory allocation
#include <linux/interrupt.h>    // tasklet
#include <linux/timer.h>        // timer

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("thodnev & Kotov");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static unsigned long *assert_array;
static int tim_cnt = 5;                     // default of timer cnt value
static unsigned long tim_delay = 10;        // default of timer delay value
static int tim_assert_count = 0;

module_param(tim_cnt, int, 0000);
MODULE_PARM_DESC(tim_cnt, "amount of timer ticks");
module_param(tim_delay, ulong, 0000);
MODULE_PARM_DESC(tim_delay, "delay between two ticks of timer");

static void timer_callback(struct timer_list * data);           // timer_callback function for timer handling
DEFINE_TIMER(test_timer, timer_callback);                       // Using this method the kernel will create the structure timer_list by itself

static void timer_callback()
{
        assert_array[tim_assert_count] = jiffies;
        tim_assert_count++;

        if (tim_assert_count < tim_cnt) {
                mod_timer(&test_timer, jiffies + tim_delay);
        }
}

static void tasklet_handler(struct tasklet_struct * data);      // tasklet_handler function
DECLARE_TASKLET(test_tasklet, tasklet_handler);                 // Using this method the kernel will create the structure tasklet_list by itself

static void tasklet_handler(struct tasklet_struct * data)
{
        pr_info("Tasklet: jiffies = %lu\n", jiffies);
}

static int __init my_module_init(void)
{
        pr_info("Mod Init: jiffies = %lu\n", jiffies);
        tasklet_schedule(&test_tasklet);

        if (tim_delay == 0) {
                pr_warn("Mod init: delay = 0\n");
        }

        if (tim_cnt <= 0) {
                pr_err("Mod Init: cnt <= 0\n");
                return -1;
        }

        assert_array = kzalloc(sizeof(assert_array) * tim_cnt, GFP_KERNEL);
        mod_timer(&test_timer, jiffies + tim_delay);
        return 0;
}

static void __exit my_module_exit(void)
{
        pr_info("Mod Exit: jiffies = %lu\n", jiffies);
        tasklet_kill(&test_tasklet);

        if (timer_pending(&test_timer)) {
                pr_warn("Exit called while timer is pending\n");
        }

        del_timer(&test_timer);
        pr_info("Timer Result: ");

        int i;
        for (i = 0; i < tim_cnt; i++) {
             if (assert_array[i] != 0) {
                pr_info("[%i] = [%lu]\n",i, assert_array[i]);
             }
        }

        if (NULL != assert_array) {
                kfree(assert_array);
        }
}

module_init(my_module_init);
module_exit(my_module_exit);
