#ifndef HIVE_MOD_  
#define HIVE_MOD_

static atomic_t *create_lock(void);
static void lock(atomic_t *argument);
static void unlock(atomic_t *argument);
static int thread_func(void *argument);
static void listtest_show_list(void);
void delete_list(void);
static int __init create_list_init(void);
static void __exit create_list_exit(void);
#endif