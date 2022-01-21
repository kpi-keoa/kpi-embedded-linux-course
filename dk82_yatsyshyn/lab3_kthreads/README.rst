Лабораторна робота 3_kthreads
=============================

Завдання
--------
Написати модуль ядра, який виконує наступні дії:

 Приймає три параметри thread_num, thread_inccnt та thread_delay
 Запускає thread_num потоків на одночасне виконання
 Кожен поток інкрементує глобальну змінну glob_var thread_inccnt разів із затримкою thread_delay
 Зберігає отримане значення змінної у список
 При вивантаженні модуль повинен виводити остаточне значення glob_var та вміст списку

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

.. code-block::

   / # insmod mnt/mdlb3k.ko thread_delay=0 thread_inccnt=1000 thread_num=10
   [  209.587306] mdlb3k: start
   [  209.587306] mdlb3k: thread_delay == 0
   / # rmmod mdlb3k
   [  222.731167] mdlb3k: glob_var = 10000
   [  222.746095] mdlb3k: mylist_var = 9823
   [  222.749562] mdlb3k: mylist_var = 9845
   [  222.750033] mdlb3k: mylist_var = 9879
   [  222.751352] mdlb3k: mylist_var = 9916
   [  222.751754] mdlb3k: mylist_var = 9941
   [  222.752242] mdlb3k: mylist_var = 9950
   [  222.753308] mdlb3k: mylist_var = 9983
   [  222.754267] mdlb3k: mylist_var = 9984
   [  222.754606] mdlb3k: mylist_var = 9990
   [  222.754893] mdlb3k: mylist_var = 10000
   / #


module `mdlb3_unlock`
~~~~~~~~~~~~~~~~~~~~~

.. code-block::

   / # insmod mnt/mdlb3k_unlock.ko thread_delay=0 thread_inccnt=1000 thread_num=10
   [  359.421244] mdlb3k_unlock: start
   [  359.422120] mdlb3k_unlock: thread_delay == 0
   / # rmmod mdlb3k_unlock
   [  393.904545] mdlb3k_unlock: glob_var = 9951
   [  393.906471] mdlb3k_unlock: mylist_var = 9789
   [  393.907014] mdlb3k_unlock: mylist_var = 9777
   [  393.907281] mdlb3k_unlock: mylist_var = 9858
   [  393.908704] mdlb3k_unlock: mylist_var = 9881
   [  393.909006] mdlb3k_unlock: mylist_var = 9873
   [  393.909648] mdlb3k_unlock: mylist_var = 9913
   [  393.910776] mdlb3k_unlock: mylist_var = 9939
   [  393.912005] mdlb3k_unlock: mylist_var = 9944
   [  393.913236] mdlb3k_unlock: mylist_var = 9949
   [  393.914475] mdlb3k_unlock: mylist_var = 9951
   / #


module `mdlb3_mylock`
~~~~~~~~~~~~~~~~~~~~~

.. code-block::

   / # insmod mnt/mdlb3k_unlock.ko thread_delay=0 thread_inccnt=1000 thread_num=10
   [  570.499064] random: crng init done
   [  570.554091] mdlb3k_mylock: start
   [  570.556452] mdlb3k_mylock: thread_delay == 0
   / # rmmod mdlb3k_mylock
   [  580.679102] mdlb3k_mylock: glob_var = 10000
   [  580.680933] mdlb3k_mylock: mylist_var = 9712
   [  580.682395] mdlb3k_mylock: mylist_var = 9792
   [  580.683050] mdlb3k_mylock: mylist_var = 9925
   [  580.683763] mdlb3k_mylock: mylist_var = 9948
   [  580.684268] mdlb3k_mylock: mylist_var = 9950
   [  580.685082] mdlb3k_mylock: mylist_var = 9964
   [  580.685924] mdlb3k_mylock: mylist_var = 9984
   [  580.686402] mdlb3k_mylock: mylist_var = 9996
   [  580.686690] mdlb3k_mylock: mylist_var = 9998
   [  580.688004] mdlb3k_mylock: mylist_var = 10000
   / #

Висновок
--------
В даній роботі я розібрався з потоками і списками в ядрі linux.
Реалізував модуль який перевіряє на коректність роботи при використанні блокування `mutex`.

