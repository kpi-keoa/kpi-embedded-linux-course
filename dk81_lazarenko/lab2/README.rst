============================================================
**Лабораторна робота №2 Використання таймерів та тасклетів**
============================================================

Завдання:
---------

Написати модуль ядра, який:
	* приймає 2 параметри *tim_cnt* і *tim_delay*;
	* виводить поточне значення *jiffies* у init та exit функціях;
	* запускає тасклет, котрий повинен надрукувати своє значення *jiffies* в лог ядра;
	* виділяє масив розміру *tim_cnt*, використовуючи динамічну алокацію;
	* запускає timer, при спрацюванні якого у масив буде записуватися поточне значення jiffies.

Хід роботи
--------------

Cпершу було створено два параметри *tim_cnt* і *tim_delay* за допомогою: -`module_param(),` -`MODULE_PARM_DESC`.

Наступним кроком було створено таймер *test_timer* за допомогою макроса *DEFINE_TIMER* та його функцію обробник *timer_callback*. У функції обробнику значення *jiffies* буде записуватися *tim_cnt* разів до масиву *assert_array*.

.. code-block:: bash
 static void timer_callback(struct timer_list * data);
 DEFINE_TIMER(test_timer, timer_callback);
 static void timer_callback()
 {
        assert_array[tim_assert_count] = jiffies;
        tim_assert_count++;
        if (tim_assert_count < tim_cnt) {
                mod_timer(&test_timer, jiffies + tim_delay);
        }
 }
Далі створюємо тасклет *test_tasklet* за допомогою макроса *DECLARE_TASKLET* та його функцію обробник *tasklet_handler*. У функції обробнику значення поточного *jiffies* буде виводитися у лог ядра.
.. code-block:: bash
 static void tasklet_handler(struct tasklet_struct * data);
 DECLARE_TASKLET(test_tasklet, tasklet_handler);
 static void tasklet_handler(struct tasklet_struct * data)
 {
        pr_info("Tasklet: jiffies = %lu\n", jiffies);
 }
У функції *init* перевіряємо вхідні параметри на корректність. Якщо параметри коректні, то ініціалізуємо массив *assert_array*, використовуючи функцію *kzalloc* (тобто усі комірки цього масиву будуть встановлені нулями). Він матиме розмір *tim_cnt* в який буде записуватися значення *jiffies* при виклику функції обробника таймеру *timer_callback*. Також ініціалізуємо тасклет та запускаємо таймер. У випадку, коли парамертри не коректні, виводимо повідомлення про помилку у лог.
.. code-block:: bash
 static int __init my_module_init(void)
 {
        pr_info("Mod Init: jiffies = %lu\n", jiffies);
        tasklet_schedule(&test_tasklet);
        if (tim_delay == 0) {
                pr_warn("Mod init: delay = 0\n");
        }
        if (tim_cnt <= 0) {
                pr_err("Mod Init: cnt <= 0\n");
                return -1;
        }
        assert_array = kzalloc(sizeof(assert_array) * tim_cnt, GFP_KERNEL);
        mod_timer(&test_timer, jiffies + tim_delay);
        return 0;
 }
У функції *exit* виводимо поточне значення *jiffies* і масиву *assert_array*. Вивільняємо пам'ять масива та припиняємо роботу таймера і тасклета.
.. code-block:: bash
 static void __exit my_module_exit(void)
 {
        pr_info("Mod Exit: jiffies = %lu\n", jiffies);
        tasklet_kill(&test_tasklet);
        if (timer_pending(&test_timer)) {
                pr_warn("Exit called while timer is pending\n");
        }
        del_timer(&test_timer);
        pr_info("Timer Result: ");
        int i;
        for (i = 0; i < tim_cnt; i++) {
             if (assert_array[i] != 0) {
                pr_info("[%i] = [%lu]\n",i, assert_array[i]);
             }
        }
        if (NULL != assert_array) {
                kfree(assert_array);
        }
 }
Результат роботи створеного модуля
-----------------------------------
.. code-block::
    Якщо *tim_cnt* дорівнює 0, друкує помилку в лог ядра за допомогою `pr_err()`.
    Не створює масив і не запускає таймер.
    / # insmod /mnt/mymod_lab2.ko tim_cnt=0 tim_delay=20
    [ 3347.420433] Mod Init: jiffies = 4298014736
    [ 3347.420734] Mod Init: cnt <= 0
    [ 3347.421150] Tasklet: jiffies = 4298014737
    insmod: can't insert '/mnt/mymod_lab2.ko': Operation not permitted
    Якщо *tim_delay* дорівнює 0, друкує попередження в лог ядра за допомогою `pr_warn()`.
    Не створює масив і не запускає таймер.
    / # insmod /mnt/mymod_lab2.ko tim_cnt=5 tim_delay=0
    [ 3402.377843] Mod Init: jiffies = 4298069693
    [ 3402.378872] Mod init: delay = 0
    [ 3402.379667] Tasklet: jiffies = 4298069695
    / # rmmod mymod_lab2
    [ 3406.252564] Mod Exit: jiffies = 4298073568
    [ 3406.253118] Timer Result:
    [ 3406.254204] [0] = [4298069697]
    [ 3406.256008] [1] = [4298069698]
    [ 3406.256700] [2] = [4298069699]
    [ 3406.256964] [3] = [4298069700]
    [ 3406.258374] [4] = [4298069702]
    Якщо модуль вигрузити раніше, ніж таймер встигне відпрацювати повністю, друкує попередження в лог ядра за допомогою `pr_warn()`.
    / # insmod /mnt/mymod_lab2.ko tim_cnt=500 tim_delay=100
    [ 3577.578364] Mod Init: jiffies = 4298244894
    [ 3577.579021] Tasklet: jiffies = 4298244894
    / # rmmod mymod_lab2
    [ 3578.903558] Mod Exit: jiffies = 4298246219
    [ 3578.903879] Exit called while timer is pending
    [ 3578.904337] Timer Result:
    [ 3578.904382] [0] = [4298245097]
    [ 3578.904653] [1] = [4298245212]
    [ 3578.904767] [2] = [4298245326]
    [ 3578.904874] [3] = [4298245432]
    [ 3578.905590] [4] = [4298245683]
    [ 3578.905725] [5] = [4298245829]
    [ 3578.906137] [6] = [4298245936]
    [ 3578.906352] [7] = [4298246067]
    [ 3578.906629] [8] = [4298246168]
