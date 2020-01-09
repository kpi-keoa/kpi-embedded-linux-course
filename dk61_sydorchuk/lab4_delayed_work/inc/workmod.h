#ifndef WORK_MOD_  
#define WORK_MOD_

static void listtest_show_list(struct list_head *a_list);
static int first_thread_func(void *argument);
static int second_thread_func(void *argument);
void work_handler(struct work_struct *arg);
void timer_handler(struct timer_list *tim);
void delete_list(struct list_head *a_list);
static int __init create_list_init(void);
static void __exit create_list_exit(void);
#endif