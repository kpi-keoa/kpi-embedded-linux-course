#include <linux/module.h>       // required by all modules
#include <linux/kernel.h>       // required for sysinfo
#include <linux/init.h>         // used by module_init, module_exit macros
#include <linux/jiffies.h>      // where jiffies and its helpers reside
#include <linux/interrupt.h>    // tasklet
#include <linux/slab.h>         // memory allocation
#include <linux/kthread.h>      // threads
#include <linux/list.h>         // list
#include <linux/sched/task.h>


MODULE_DESCRIPTION("Threads");
MODULE_AUTHOR("artem-dk81");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");          // this affects the kernel behavior


static unsigned long thread_num = 5;    // Amount of simultaneous threads
static unsigned long thread_inccnt = 10; // How many times thread will increment glob_var
static unsigned long thread_delay = 100; // Delay between increment of glob_var
static LIST_HEAD(glob_var_list); //create inst. of struct_list
static unsigned long glob_var;


module_param(thread_num, ulong, 0000);
MODULE_PARM_DESC(thread_num, "Amount of threads");

module_param(thread_inccnt, ulong, 0000);
MODULE_PARM_DESC(thread_inccnt, "How many times thread will increment glob_var");

module_param(thread_delay, ulong, 0000);
MODULE_PARM_DESC(thread_delay, "Delay between increment of glob_var");



struct my_list_struct{     		//node content
        struct list_head list;
        typeof(glob_var) glob_var_saved;
};

static struct task_struct **thread = NULL; // kernel stores the list of processes here

static int my_thread_handler(void *data)
{
        unsigned long *glob_var_param = data; // copy of data param
        unsigned long temp_cnt = thread_inccnt;

         pr_info("%s: Thread enetered handler with temp_cnt = %lu,\n"
                 "\t\t glob_var_param = %lu\n",
                 module_name(THIS_MODULE), temp_cnt, *glob_var_param);

        struct my_list_struct *new_node = kzalloc(sizeof(*new_node), GFP_KERNEL);
        if (NULL == new_node) {
                pr_err("Can`t allocate memory for new node of list\n");
                return -1;
        }

        INIT_LIST_HEAD(&new_node->list);

        while (!kthread_should_stop() && temp_cnt > 0){
                new_node->glob_var_saved = ++(*glob_var_param);
                temp_cnt--;
                schedule_timeout_uninterruptible(msecs_to_jiffies(thread_delay));
                pr_info("%s: Thread glob_var: %lu\n", module_name(THIS_MODULE), new_node->glob_var_saved);
        }
	
        list_add_tail(&new_node->list,&glob_var_list); // add new node to glob_var_list
		
        return 0;
}

static int __init mod3_init(void)
{

        glob_var = 0;
        int i;

        pr_info("%s started with: thread_num = %lu,\n "
                "\t\t thread_inccnt = %lu\n "
                "\t\t thread_delay = %lu\n ",
                module_name(THIS_MODULE),thread_num,thread_inccnt, thread_delay);

        if (0 == thread_num) {
                pr_err("%s: Invalid thread num  0\n", module_name(THIS_MODULE));
                return -1;
        }

        if (0 == thread_delay) {
                pr_warn("Thread delay is 0\n");
        }

        if (0 == thread_inccnt) {
                pr_warn("Thread increment count variable is 0\n");
        }

        thread = kmalloc(sizeof(**thread)*thread_num, GFP_KERNEL);

        for (i = 0; i < thread_num; i++) {
                thread[i] = kthread_run(my_thread_handler, &glob_var,"thread[%i]", i);

                if (IS_ERR(thread[i])) {
                        pr_err("Thread creation error %s\n", PTR_ERR(thread[i]) == -ENOMEM ? "ENOMEM" : "EINTR");
                        thread[i] = NULL;
                }

                get_task_struct(thread[i]);  
                pr_info("%s: %i connected\n",module_name(THIS_MODULE),thread[i]->pid);
        }
        return 0;
}

static void __exit mod3_exit(void)
{
        if (NULL == thread){
                pr_warn("%s: Memory for thread was not allocated\n", module_name(THIS_MODULE));
        }

        int i;
        for (i = 0; i < thread_num; i++) {
                kthread_stop(thread[i]);
                put_task_struct(thread[i]); // deallocate the slab cache containing the task_struct
        }
        kfree(thread);

        struct my_list_struct *cursor, *next;

        i = 0;
        list_for_each_entry_safe (cursor, next, &glob_var_list, list) { //  iterates backward through the list
                pr_info("%s: node[%i] glob_var_saved = %lu\n",  module_name(THIS_MODULE), i, cursor->glob_var_saved);
                i++;
                list_del(&cursor->list);
                kfree(cursor);
        }
	
	pr_info("%s: glob_var = %lu\n",  module_name(THIS_MODULE), glob_var);
	

        return;
}

module_init(mod3_init);
module_exit(mod3_exit);