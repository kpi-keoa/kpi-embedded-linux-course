#include <linux/module.h>       // required by all modules
#include <linux/kernel.h>       // required for sysinfo
#include <linux/init.h>         // used by module_init, module_exit macros
#include <linux/jiffies.h>      // where jiffies and its helpers reside
#include <linux/interrupt.h>    // tasklet
#include <linux/timer.h>        // timer
#include <linux/slab.h>         // memory allocation

MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
MODULE_AUTHOR("Vitalii Romakh");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");          // this affects the kernel behavior

static int cnt = 2;                     // default value
static unsigned long delay = 10;        // default value
static int timer_assert_count = 0;
static unsigned long *assert_array;

module_param(cnt, int, 0000);
MODULE_PARM_DESC(cnt, "A variable defining number of times for timer to assert");

module_param(delay, ulong, 0000);
MODULE_PARM_DESC(delay, "A variable defining from which value timer will countdown");

static void study_timer_callback(struct timer_list * data);             // timer_callback function for timer handling
DEFINE_TIMER(study_timer, study_timer_callback);                        // Using this method the kernel will create the structure timer_list by itself

static void study_tasklet_handler(struct tasklet_struct * data);
DECLARE_TASKLET(study_tasklet, study_tasklet_handler);

static int __init mod2_lab_init(void)
{

        pr_info("Mod Init: jiffies = %lu\n", jiffies);     // Alias function for printk(KERN_INFO "");

        tasklet_schedule(&study_tasklet);       // shedule tasklet with regular priority

        if (0 == delay){
                pr_warn("Mod init: delay = 0\n");
        }

        if (cnt <= 0){
                pr_err("Mod Init: cnt <= 0\n");      // Alias function for printk(KERN_ERR "");
                return -1;
        }

        assert_array = kzalloc(sizeof(assert_array) * cnt, GFP_KERNEL);    // allocate memory and set it to zero

        mod_timer(&study_timer, jiffies + delay);

        return 0;
}
static void study_tasklet_handler(struct tasklet_struct * data)
{
        pr_info("Tasklet: jiffies = %lu\n", jiffies);       // allias
}

static void study_timer_callback()
{
        assert_array[timer_assert_count] = jiffies;
        timer_assert_count++;

        if (timer_assert_count < cnt){
                mod_timer(&study_timer, jiffies + delay);       // reload timer
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
        for(i = 0; i < cnt; i++){
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
