============================================================
**Лабораторна робота №3 Потоки ядра**
============================================================

Завдання:
---------

Написати модуль ядра, який:
	* приймає 3 параметри *thread_num*, *thread_inccnt* та *thread_delay*;
	* запускає *thread_num* потоків на одночасне виконання;
	* кожен поток інкрементує глобальну змінну *glob_var* (*thread_inccnt* разів) із затримкою *thread_delay*;
	* зберігає отримане значення змінної у список;
	* при вивантаженні модуль повинен виводити остаточне значення *glob_var* та вміст списку.

Хід роботи
--------------

Спершу було створено список *glob_var_list*, масив потоків *thread* та структуру *my_list_struct*, яка наслідує структуру *list_head* та зберігатиме значення змінної *glob_var*.

.. code-block:: bash

 static LIST_HEAD(glob_var_list);

 static struct task_struct **thread = NULL;
 static unsigned long glob_var;

 struct my_list_struct{
        struct list_head list;
        typeof(glob_var) glob_var_saved;
 };

Далі створюємо функцію *thread_func*, яка виконуватиметься у потоці. Для перевірки чи необхідно завершувати поток чи ні, використовується *kthread_should_stop()*. Затримка, тривалістю *thread_delay*, створюється за допомогою функції *schedule_timeout_uninterruptible()*.


.. code-block:: bash

 static int thread_func(void *param)
 {
        unsigned long *glob_var_param = param;
        unsigned long temp_cnt = thread_inccnt;

        struct my_list_struct *new_node = kzalloc(sizeof(*new_node), GFP_KERNEL);
        if (NULL == new_node) {
                pr_err("Can`t allocate memory for new node of list\n");
                return -1;
        }

        INIT_LIST_HEAD(&new_node->list);

        while (!kthread_should_stop() && temp_cnt > 0) {
                new_node->glob_var_saved = ++(*glob_var_param);
                temp_cnt--;
                schedule_timeout_uninterruptible(msecs_to_jiffies(thread_delay));
                pr_info("%s: thread glob_var: %lu\n", module_name(THIS_MODULE), new_node->glob_var_saved);
        }

        list_add_tail(&(new_node->list),&glob_var_list);
        return 0;
 }

У функції *__init my_module_init* було написано блок перевірки правильності значень уведених параметрів модуля та виділення пам'яті під масив *thread*. Також відбувається запуск кожного з потоків, використовуючи функцію
*kthread_run()*, у неї передається обробник потока *thread_func* та указатель на дані, які передаються в цей обробник.

.. code-block:: bash

 static int __init my_module_init(void)
 {
        int i;

        glob_var = 0;
        pr_info("%s started with: thread_num = %lu,\n"
                "\t\t   thread_inccnt = %lu,\n"
                "\t\t   thread_delay = %lu.\n",
                module_name(THIS_MODULE),thread_num,thread_inccnt, thread_delay);

        if (0 == thread_num) {
                pr_err("%s: thread num shouldn`t be 0\n", module_name(THIS_MODULE));
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

                thread[i] = kthread_run(thread_func, &glob_var, "thread[%i]", i);
                pr_info("%s: thread #%i connected\n",module_name(THIS_MODULE), i+1);

                if (IS_ERR(thread[i])) {
                        pr_err("Thread creation error %s\n", PTR_ERR(thread[i]) == -ENOMEM ? "ENOMEM" : "EINTR");
                        thread[i] = NULL;
                }

                get_task_struct(thread[i]);
        }
        return 0;
 }

У функції *__exit my_module_exit* виконується зупинка потоків використовуючи *kthread_stop()*, вивільняється пам'ять масива *thread*, виводиться вміст списку *glob_var_list*, а після виконується його очищення.

.. code-block:: bash

 static void __exit my_module_exit(void)
 {
        if (NULL == thread) {
                pr_warn("%s: memory for thread was not allocated\n", module_name(THIS_MODULE));
        }

        int i;
        for (i = 0; i < thread_num; i++) {
                kthread_stop(thread[i]);
                put_task_struct(thread[i]);
        }

        kfree(thread);
        struct my_list_struct *cursor, *next;

        i = 0;
        list_for_each_entry_safe (cursor, next, &glob_var_list, list) {
                pr_info("%s: node[%i] glob_var_saved = %lu\n",  module_name(THIS_MODULE), i, cursor->glob_var_saved);
                i++;
                list_del(&cursor->list);
                kfree(cursor);
        }

        return;
 }

Для створення, версії з синхронізацією, функції обробника для потока *thread_func* використовується *mutex*. Його задачею є захист об'єкта від доступу до нього іншими потоками. Використувуючи макрос *DEFINE_MUTEX*
виконується створення змінної для контролю доступа до об'єкту. Для блокування і розблокування об'єкту, використовують *mutex_lock()* та *mutex_unlock()*, відповідно.

Результат роботи створеного модуля
-----------------------------------

.. code-block::

 Результат роботи модуля, без уведених аргументів. За замовчуванням, кількість потоків рівна 1, кількість інкрементацій 10,
 затримка рівна 10 мілісекунд.
    / # insmod /mnt/uns_threads.ko
    [   38.525153] uns_threads: loading out-of-tree module taints kernel.
    [   38.540437] uns_threads started with: thread_num = 1,
    [   38.540437]             thread_inccnt = 10,
    [   38.540437]             thread_delay = 10.
    [   38.548472] uns_threads: thread #1 connected
    [   38.557962] insmod (90) used greatest stack depth: 13680 bytes left
    [   38.565393] uns_threads: thread glob_var: 1
    [   38.582077] uns_threads: thread glob_var: 2
    [   38.604703] uns_threads: thread glob_var: 3
    [   38.617276] uns_threads: thread glob_var: 4
    [   38.633105] uns_threads: thread glob_var: 5
    [   38.644791] uns_threads: thread glob_var: 6
    [   38.659603] uns_threads: thread glob_var: 7
    [   38.671584] uns_threads: thread glob_var: 8
    [   38.683487] uns_threads: thread glob_var: 9
    [   38.696482] uns_threads: thread glob_var: 10
    / # rmmod uns_threads
    [  141.219111] uns_threads: node[0] glob_var_saved = 10

 Результат роботи модуля без синхронізації, уведенні значення: thread_num = 10, thread_inccnt = 600, thread_delay = 0.
 Результат повинен бути рівний 6000.

    / # insmod /mnt/uns_threads.ko thread_num=10 thread_inccnt=600 thread_delay=0
    / # rmmod uns_threads
    [  120.464786] uns_threads: node[0] glob_var_saved = 5714
    [  120.466801] uns_threads: node[1] glob_var_saved = 5820
    [  120.473773] uns_threads: node[2] glob_var_saved = 5816
    [  120.476525] uns_threads: node[3] glob_var_saved = 5862
    [  120.477444] uns_threads: node[4] glob_var_saved = 5893
    [  120.480560] uns_threads: node[5] glob_var_saved = 5908
    [  120.485772] uns_threads: node[6] glob_var_saved = 5962
    [  120.489067] uns_threads: node[7] glob_var_saved = 5969
    [  120.489924] uns_threads: node[8] glob_var_saved = 5973
    [  120.492529] uns_threads: node[9] glob_var_saved = 5998

 Як видно результат рівний 5998. Він не збігається, з очікуваним значенням.

 Результат роботи модуля з синхронізацією, уведенні значення: thread_num = 10, thread_inccnt = 600,
 thread_delay = 0. Результат повинен бути рівний 6000.

    / # insmod /mnt/syn_threads.ko thread_num=10 thread_inccnt=600 thread_delay=0
    / # rmmod syn_threads
    [  318.085184] syn_threads: node[0] glob_var_saved = 5683
    [  318.085938] syn_threads: node[1] glob_var_saved = 5700
    [  318.086431] syn_threads: node[2] glob_var_saved = 5784
    [  318.086781] syn_threads: node[3] glob_var_saved = 5815
    [  318.087253] syn_threads: node[4] glob_var_saved = 5840
    [  318.087662] syn_threads: node[5] glob_var_saved = 5915
    [  318.088068] syn_threads: node[6] glob_var_saved = 5979
    [  318.088590] syn_threads: node[7] glob_var_saved = 5984
    [  318.089034] syn_threads: node[8] glob_var_saved = 5998
    [  318.089519] syn_threads: node[9] glob_var_saved = 6000

 Як видно результат рівний 6000. Він збігається, з очікуваним значенням.

 Результат роботи модуля з власними реалізаціями lock(), unlock(). Уведенні значення: thread_num = 10, thread_inccnt = 600,
 thread_delay = 0. Результат повинен бути рівний 6000.

    / # insmod /mnt/my_threads.ko thread_num=10 thread_inccnt=600 thread_delay=0
    / # rmmod my_threads
    [  646.699718] my_threads: node[0] glob_var_saved = 5681
    [  646.701314] my_threads: node[1] glob_var_saved = 5785
    [  646.702267] my_threads: node[2] glob_var_saved = 5798
    [  646.702267] my_threads: node[3] glob_var_saved = 5902
    [  646.706110] my_threads: node[4] glob_var_saved = 5920
    [  646.709575] my_threads: node[5] glob_var_saved = 5922
    [  646.711331] my_threads: node[6] glob_var_saved = 5934
    [  646.712968] my_threads: node[7] glob_var_saved = 5952
    [  646.713923] my_threads: node[8] glob_var_saved = 5961
    [  646.716475] my_threads: node[9] glob_var_saved = 6000

 Як видно результат рівний 6000. Він збігається, з очікуваним значенням.


Висновки:
-------------

В даній лабораторній роботі було досліджено потоки, як їх запускати і зупиняти. Написано модуль, який створює *thread_num* потоків, кожен з яких інкрементує значення глобальної змінної *glob_var* (*thread_inccnt* разів) з затримкою в *thread_delay*. В результаті перевіркі модуля було з'ясовано, що при наявній синхронізації модуль працює стабільно та коректно, а при її відсутності з'являються не вірні значення.
Також було перевірено модуль з власними реалізаціями *lock()* та *unlock()*, він працює коректно.
