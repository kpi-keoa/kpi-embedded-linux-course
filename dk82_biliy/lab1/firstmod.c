#include <linux/module.h>	
#include <linux/moduleparam.h>
#include <linux/kernel.h>	
#include <linux/init.h>		
#include <linux/jiffies.h>	

MODULE_DESCRIPTION("Basic module which shows user_name parameter, and worktime");
MODULE_AUTHOR("vitaliy2034");
MODULE_VERSION("0.2");
MODULE_LICENSE("GPL");

#define NAME_BUF_LEN    15
#define DEF_USER_NAME   "$username"

static char usr_name[NAME_BUF_LEN] = {0};
module_param_string(user_name, usr_name, NAME_BUF_LEN, 0664);
MODULE_PARM_DESC(usr_name, "User name. Should be specified");

static uint64_t init_jiffies_64 = 0;

static int __init firstmod_init(void)
{
    char * _user_name; 
    
    //Set jiffies at module initialization
    init_jiffies_64 = get_jiffies_64();
    
    //Check is name param spcified
    if(*usr_name)
    {
        _user_name = usr_name;
    }
    else
    {
        _user_name = DEF_USER_NAME;
        pr_warn("user_name not specified\n");
    }
	pr_info("user_name: %s\n", _user_name);
	return 0;
}
 
static void __exit firstmod_exit(void)
{
    uint64_t total_time_64 = get_jiffies_64() - init_jiffies_64;
    
	pr_info("Stop module\n Module works: %llu ms", jiffies64_to_msecs(total_time_64));
}
 
module_init(firstmod_init);
module_exit(firstmod_exit);
