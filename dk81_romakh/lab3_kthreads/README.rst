Лабораторна робота 3
====================

Завдання:
---------
Написати модуль який:
 - містить глобальну змінну **glob_var**;
 - приймає три аргументи:
        - **thread_num** - кількість потоків для одночасного виконання;
        - **thread_inccnt** - кількість інкрементів **glob_var** для кожного потоку;
        - **thread_delay** - затримка між кожним інкрементом **glob_var** у потоці;
 - запускає **thread_num** потоків на одночасне виконання;
 - кожен потік інкрементує змінну **glob_var** **thread_inccnt** разів з затримкою **thread_delay** (в випадку **thread_delay** = 0 - без затримки), додає значення змінної **glob_var** до списку і завершується;
 - при вивантажені модуль виводить значення **glob_var** та вміст списку;
 - для змінних, списку та потоків використовувати динамічну аллокацію; змінну передавати в потік в якості аргументу;
 - передбачити можливість завчасного вивантаження.


 Хід роботи
----------

Було створено список glob_var_list, який містить в собі структуру my_list_struct та масив потоків thread.

    .. code-block:: bash

    static LIST_HEAD(glob_var_list);

    struct my_list_struct{
            struct list_head list;
            typeof(glob_var) glob_var_saved;
    };

    static struct task_struct **thread = NULL;

Створено функцію обробник для потока thread_func (для прикладу наведено версію без синхронізації)

    .. code-block:: bash

    static int thread_func(void *data)
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

            list_add_tail(&(new_node->list),&glob_var_list);

            return 0;
    }

**Потоки**

Потоки були створені за допомогою kthreads. Для цього:
    1) Виділили пам'ять під масив thread типу *task_struct*
    2) Запустити потоки за добомогою *kthread_run()*, в цій функції необхідно передати функцію обробник потока *thread_func*
    3) Для створення затримки у функції обробнику використовуємо *schedule_timeout_uninterruptible()*
    4) У функції обробнику *thread_func* для перевірки чи необхідно завершувати поток чи ні, використовуємо *kthread_should_stop()*
    5) Для зупинки потоку використовуємо *kthread_stop()*


**Синхронізація**
    Мьютекс - задачею мьютекса є захист об'єкта від доступа до нього іншими потоками, відмінних від того що захопив мьютекс.

    1) Створити змінну для контролю доступу до об'єкта за допомогою макросу *DEFINE_MUTEX*
    2) Для блокування об'єкта *mutex_lock()*
    3) Розблокування *mutex_unlock()*

**Результат роботи**

**1) Результат роботи модуля без синхронізації**
Запустимо модуль з такими параметрами: thread_num=10 thread_inccnt=2000 thread_delay=0. Після того як модуль відпрацює ми очікуємо, що значення змінної glob_var буде дорівнювати 2000.

    .. code-block:: bash

    / # insmod /mnt/un_synch.ko thread_num=10 thread_inccnt=2000 thread_delay=0
    / # rmmod un_synch
    [ 1962.366597] un_synch: node[0] glob_var_saved = 19325
    [ 1962.367891] un_synch: node[1] glob_var_saved = 19743
    [ 1962.368894] un_synch: node[2] glob_var_saved = 19849
    [ 1962.369782] un_synch: node[3] glob_var_saved = 19852
    [ 1962.370682] un_synch: node[4] glob_var_saved = 19896
    [ 1962.371719] un_synch: node[5] glob_var_saved = 19908
    [ 1962.372619] un_synch: node[6] glob_var_saved = 19957
    [ 1962.373908] un_synch: node[7] glob_var_saved = 19962
    [ 1962.375353] un_synch: node[8] glob_var_saved = 19982
    [ 1962.375723] un_synch: node[9] glob_var_saved = 19985

Як бачимо, без синхронізації результат вийшов неправильним.


**2) Результат роботи модуля із синхронізацією на мьютексах**
Запустимо модуль з такими параметрами: thread_num=10 thread_inccnt=2000 thread_delay=0. Після того як модуль відпрацює ми очікуємо, що значення змінної glob_var буде дорівнювати 2000.

    .. code-block:: bash

    / # insmod /mnt/synch.ko thread_num=10 thread_inccnt=2000 thread_delay=0
    / # rmmod synch
    [ 2268.339447] synch: node[0] glob_var_saved = 19379
    [ 2268.340520] synch: node[1] glob_var_saved = 19694
    [ 2268.341106] synch: node[2] glob_var_saved = 19802
    [ 2268.342422] synch: node[3] glob_var_saved = 19894
    [ 2268.343564] synch: node[4] glob_var_saved = 19905
    [ 2268.344066] synch: node[5] glob_var_saved = 19914
    [ 2268.344964] synch: node[6] glob_var_saved = 19958
    [ 2268.345896] synch: node[7] glob_var_saved = 19983
    [ 2268.346902] synch: node[8] glob_var_saved = 19991
    [ 2268.347688] synch: node[9] glob_var_saved = 20000


Як бачимо, результат вийшов правильним.

**3) Результат роботи модуля із власною синхронізацією за допомогою атомарних операцій**
Запустимо модуль з такими параметрами: thread_num=10 thread_inccnt=2000 thread_delay=0. Після того як модуль відпрацює ми очікуємо, що значення змінної glob_var буде дорівнювати 2000.

    .. code-block:: bash

    / # insmod /mnt/my_synch.ko thread_num=10 thread_inccnt=2000 thread_delay=0
    / # rmmod my_synch
    [ 2467.893353] my_synch: node[0] glob_var_saved = 19574
    [ 2467.894145] my_synch: node[1] glob_var_saved = 19576
    [ 2467.894642] my_synch: node[2] glob_var_saved = 19782
    [ 2467.895268] my_synch: node[3] glob_var_saved = 19784
    [ 2467.895688] my_synch: node[4] glob_var_saved = 19864
    [ 2467.896216] my_synch: node[5] glob_var_saved = 19901
    [ 2467.896229] my_synch: node[6] glob_var_saved = 19925
    [ 2467.896229] my_synch: node[7] glob_var_saved = 19984
    [ 2467.897815] my_synch: node[8] glob_var_saved = 19990
    [ 2467.898630] my_synch: node[9] glob_var_saved = 20000

Як бачимо, результат вийшов правильним.
