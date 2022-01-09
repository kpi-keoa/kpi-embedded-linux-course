#include <linux/module.h>       // required by all modules
#include <linux/kernel.h>       // required for sysinfo
#include <linux/init.h>         // used by module_init, module_exit macros
#include <linux/jiffies.h>      // where jiffies and its helpers reside
#include <linux/interrupt.h>    // tasklet
#include <linux/timer.h>        // timer
#include <linux/slab.h>         // memory allocation

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("thodnev, Nysh Yevhen");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");          // this affects the kernel behavior

static int t_cnt = 4 ;                     // default value
static unsigned long t_delay = 10;        // default value
static int t_assert_count = 0;
static unsigned long *assert_array;

module_param(t_cnt, int, 0000);
MODULE_PARM_DESC(t_cnt, "variable that determines the number of clock cycles timer");

module_param(t_delay, ulong, 0000);
MODULE_PARM_DESC(t_delay, "a variable that determines the timer t_delay");

static void t_callback(struct timer_list * data);             // timer_callback function for timer handling
DEFINE_TIMER(study_timer, t_callback);                        // Using this method the kernel will create the structure timer_list by itself

static void tasklet_handler(struct tasklet_struct * data);
DECLARE_TASKLET(study_tasklet, tasklet_handler);

static int __init mod2_lab_init(void)
{

        pr_info("Mod Init: jiffies = %lu\n", jiffies);     // Alias function for printk(KERN_INFO "");

        tasklet_schedule(&study_tasklet);       // shedule tasklet with regular priority

        if (0 == t_delay){
                pr_warn("Mod init: t_delay = 0\n");
        }

        if (t_cnt <= 0){
                pr_err("Mod Init: t_cnt <= 0\n");      // Alias function for printk(KERN_ERR "");
                return -1;
        }

        assert_array = kzalloc(sizeof(assert_array) * t_cnt, GFP_KERNEL);    // allocate memory and set it to zero

        mod_timer(&study_timer, jiffies + t_delay);

        return 0;
}
static void tasklet_handler(struct tasklet_struct * data)
{
        pr_info("Tasklet: jiffies = %lu\n", jiffies);       // allias
}

static void t_callback()
{
        assert_array[t_assert_count] = jiffies;
        t_assert_count++;

        if (t_assert_count < t_cnt){
                mod_timer(&study_timer, jiffies + t_delay);       // reload timer
        }
}

static void __exit mod2_lab_exit(void)
{
        pr_info("Mod Exit: jiffies = %lu\n", jiffies);

        tasklet_kill(&study_tasklet);

        if (timer_pending(&study_timer)){
                pr_warn("Exit called while timer is pending\n");
        }

        del_timer(&study_timer);


        pr_info("Timer Result: ");

        int i;

        for(i = 0; i < t_cnt; i++){
             if(assert_array[i] != 0){
                pr_info("[%i] = [%lu]\n",i, assert_array[i]);
             }
        }

        if (NULL != assert_array){
                kfree(assert_array);
        }
}

module_init(mod2_lab_init);
module_exit(mod2_lab_exit);
