===========================
**Lab2: Simple_kernel_module**
===========================

**Description**

There we have an instance of kernel module. 
It has privileges compare to userspace-applications.
You can't use standard 	libraries, 'cause modules need to be fast and light. 
It is not safe to insert and remove modules, unless if you know what you are doing. 


**Explaining**

After including all linux libraries that we need to use we define module license as "Dual MIT/GPL" this disabling lock debugging due to kernel taint. 
Execution starts from the function ``module_init(<init_func>)`` and end with ``module_exit(<exit_func>);``
Result of ``printk()`` is a warning message that notify us about inserting this module to the kernel and print ``jiffies`` - amount of interruptions that happened from the system boot. 


The task was to implement tasklet, that print the same variable ``jiffies`` and compare results of execution of this code on x86 and BBXM. Also add opportunity to set value of the username for system's greeting from command line during inserting the module.  


Tasklets are a deferral scheme that you can schedule for a registered function to run later. They are atomic that fact exclude using primitives sync and  their priority (bottom-half) is less than ISR(top-half).
Their structure defined in the <linux/interrupt.h>. 
It is usefull to use it for    

**Implementing**
We created tasklet with a static method by MACROS ``DECLARE_TASKLET(name, func, data);`` which is equivalent for the next declaration:
``struct tasklet_struct my_tasklet = {

 NULL, 0, ATOMIC_INIT(0), tasklet_handler, dev

};``
We created tasklet called ``my_tasklet``, where attribute ``count = 0``, it means that tasklet has approval for execution. The ``tasklet_handler`` wiil be handler of this tasklet and it receive argument `dev`` as input value. 
For dynamic initializition use ``struct tasklet_struct* t`` and call it by ``tasklet_init(t, tasklet_handler, dev)``

For schedule this tasklet we are put him in the queue (implemented as linked list) with ``tasklet_schedule(&my_tasklet); //waiting for running``
After execution of this function we are need to remove it from the queue with ``tasklet_kill(&my_tasklet)``

We are saving input username 
static char *user = "$username"; //set user=<name> in the terminal 
module_param(user, charp, S_IRUGO);

**Module assembly and testing**
For building that module use ``make``
(Next commands need **root-access**)
For inserting use ``insmod <module_name>``
For removing use ``rmmod <module_name>``
For showing kernel's log use ``dmesg -k``

**Results**
That value mean amount interruptions per second to the kernel.
The value ``jiffies`` that we've got: 
``
[14340.624038] Hello, bramory!
               Our jiffies = 4299179412
[14340.624044] Tasklets's jiffies = 4299179412
[14350.850658] Long live the Kernel! =)
[14440.754670] Hello, bramory!
               Our jiffies = 4299209451
[14440.754832] Tasklets's jiffies = 4299209451
[14444.491189] Long live the Kernel! =)

``

Calculations are showed, that we have stable amount of interruptions
``(4299209451 - 4299179412) / (14440.75467 - 14340.62403) = 299.998 Hz
`` 

With command ``zcat /proc/config.gz | grep CONFIG_HZ`` we can check that value.
For x86 ``CONFIG_HZ = 300`` and for BBXM it is less - ``CONFIG_HZ = 100``
As we see, if the time for execution of tasklet will be bigger then period of 1``jiffies`` then in that moment 1 interruption will occur and ``jiffies`` will increase by 1. That's why can exist the difference in a values ``jiffies`` that comes from the tasklet after execution on BBXM or x86. 


Pay attention to the killing unwanted tasklets and preventing that kind of situation when tasklet recursively add yourself in the schedule.



**Contacts**

You can also contact me:

**Telegram:** @Bramory <https://tg.me/Bramory>

**E-mail** <glovatskiy.dmitriy@gmail.com>
