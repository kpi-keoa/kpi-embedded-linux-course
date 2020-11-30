//based on demo by thodnev & Morel code
 
#include <linux/module.h>    // required by all modules
#include <linux/moduleparam.h>
#include <linux/kernel.h>    // required for sysinfo
#include <linux/init.h>      // used by module_init, module_exit macros
#include <linux/jiffies.h>   // where jiffies and its helpers reside
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/mm.h>

MODULE_DESCRIPTION("timers,memory");
MODULE_AUTHOR("thodnev & luchik"); 
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");	//it affects the kernel behavior

static int cnt = 0;
module_param(cnt, int, 0);
MODULE_PARM_DESC(cnt, "number of addings");

static int delay = 0;
module_param(delay, int, 0);
MODULE_PARM_DESC(cnt, "period in ms");

struct tmr_data {
    typeof(cnt) cnt;
    typeof(delay) delay;
    struct timer_list tim;
};

static struct tmr_data t_data;


static typeof(jiffies) *array = NULL;
static int cnt_t = 0;
static int i = 0;

void my_tasklet(unsigned long arg)
{
    printk(KERN_INFO "Tasklet jiffies is %lu\n", jiffies);
}

DECLARE_TASKLET(tasklet, my_tasklet, 0);

static void jif_to_array(struct timer_list *data)
{
    struct tmr_data *mydata;
    mydata = container_of(data, struct tmr_data, tim);
    
    if(cnt_t < mydata->cnt) {
        array[cnt_t] = jiffies;
        cnt_t++;
        mod_timer(&mydata->tim, jiffies + msecs_to_jiffies(mydata->delay));
    }
}

static int __init mymod_init(void)
{
    t_data.cnt = cnt;
    t_data.delay = delay;
    
    printk(KERN_INFO "Initial Jiffies is %lu\n", jiffies);
    tasklet_schedule(&tasklet);

    if ((array = kzalloc(t_data.cnt * sizeof(*array), GFP_KERNEL)) == NULL) {
        printk(KERN_ERR "Allocate error\n");
        goto fail;
    }

    if (t_data.cnt <= 0) {
        printk(KERN_ERR "Wrong cnt\n");
        goto fail;
    }

    if (t_data.delay < 0) {
        printk(KERN_ERR "Wrong delay\n");
        goto fail;
    }

    t_data.tim.expires = jiffies + msecs_to_jiffies(t_data.delay);
    timer_setup(&t_data.tim, jif_to_array, 0);
    add_timer(&t_data.tim);
    return 0;

fail:
    printk(KERN_ERR "Terminated\n");
    return -1;
}

static void __exit mymod_exit(void)
{
    printk(KERN_INFO "Exit jiffies is %lu\n", jiffies);

    del_timer_sync(&t_data.tim);
    
    if(cnt_t < t_data.cnt)
        printk(KERN_WARNING "Not passed\n");
    
    for (i = 0; i < cnt_t; i++) 
        printk(KERN_INFO "ArSray[%d] = %lu\n", i, array[i]);	

    kfree((const void *)array);
}
 
module_init(mymod_init);
module_exit(mymod_exit);

