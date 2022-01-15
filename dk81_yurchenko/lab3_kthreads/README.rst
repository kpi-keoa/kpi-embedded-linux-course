Лабораторна робота 3_kthreads
=============================

Завдання
--------
Напишите модуль ядра, который:
 - содержит глобальную переменную glob_var и параметры thread_num, thread_inccnt, thread_delay
 - запускает thread_num потоков на одновременное выполнение
 - каждый поток инкрементирует переменную glob_var thread_inccnt раз с задержкой thread_delay (может быть 0 — в этом случае, без задержки), кладет значение переменной в список и завершается
 - при выгрузке модуль выводит значение переменной glob_var и содержимое списка
 - для переменной, списка, потоков использовать динамическую аллокацию. Переменную передавать в поток аргументом
 - предусматривает возможность досрочной выгрузки и будет нормально отрабатывать в этом случае

Xiд роботи
----------

thread
~~~~~~

 - Виділяємо пам'ять для масива структур `struct task_struct`
 - Запускаємо за допомогою функцію `kthread_run`
 - Створюємо функцію яку вказили раніше у функції `kthread_run`
 - Щоб завершити потік `kthread_stop()`
 - Для перевірки на `kthread_stop()` за допомогою `kthread_should_stop()`

list
~~~~

 - LIST_HEAD() дана функцію створює структуру `list_head`
 - Функцію `list_add_tail()` додає елемент в кінець

mutex
~~~~~

 - стрворює змінну за допомогою `DEFINE_MUTEX()`;
 - `mutex_lock` блокуємо
 - `mutex_unlock` розблоковуємо

Результат
---------

module `mdlb3`
~~~~~~~~~~~~~~

З mutex

.. code-block::

   / # insmod mnt/mdlb3k.ko thread_delay=0 thread_inccnt=1000 thread_num=10
   [    7.174971] mdlb3k: start
   [    7.175293] mdlb3k: thread_delay == 0
   / # rmmod mdlb3k
   [   42.309106] mdlb3k: glob_var = 10000
   [   42.309555] mdlb3k: mylist_var = 9394
   [   42.310032] mdlb3k: mylist_var = 9661
   [   42.310467] mdlb3k: mylist_var = 9770
   [   42.310926] mdlb3k: mylist_var = 9824
   [   42.311355] mdlb3k: mylist_var = 9886
   [   42.311805] mdlb3k: mylist_var = 9907
   [   42.312236] mdlb3k: mylist_var = 9915
   [   42.312652] mdlb3k: mylist_var = 9934
   [   42.313123] mdlb3k: mylist_var = 9980
   [   42.313540] mdlb3k: mylist_var = 10000

module `mdlb3_unlock`
~~~~~~~~~~~~~~~~~~~~~

Без mutex

.. code-block::

   / # insmod mnt/mdlb3k_unlock.ko thread_delay=0 thread_inccnt=1000 thread_num=10
   [   51.974133] mdlb3k_unlock: start
   [   51.974362] mdlb3k_unlock: thread_delay == 0
   / # rmmod mdlb3k_unlock
   [   80.909147] mdlb3k_unlock: glob_var = 9979
   [   80.909663] mdlb3k_unlock: mylist_var = 8713
   [   80.910263] mdlb3k_unlock: mylist_var = 9490
   [   80.910754] mdlb3k_unlock: mylist_var = 9596
   [   80.911322] mdlb3k_unlock: mylist_var = 9770
   [   80.911864] mdlb3k_unlock: mylist_var = 9798
   [   80.912361] mdlb3k_unlock: mylist_var = 9820
   [   80.912909] mdlb3k_unlock: mylist_var = 9864
   [   80.913432] mdlb3k_unlock: mylist_var = 9886
   [   80.914012] mdlb3k_unlock: mylist_var = 9909
   [   80.914539] mdlb3k_unlock: mylist_var = 9979

Висновок
--------
В даній роботі я розібрався з потоками і списками в ядрі linux.
Реалізував модуль який перевіряє на коректність роботи при використанні блокування `mutex`.

