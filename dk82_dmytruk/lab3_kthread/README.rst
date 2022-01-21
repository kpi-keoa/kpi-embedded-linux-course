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
 - Створюємо функцію handler яку передаємо у функцію `kthread_run`
 - Створюємо і запускаємо потоки за допомогою функції `kthread_run`
 - Щоб завершити потік `kthread_stop()`
 - Для перевірки на `kthread_stop()` за допомогою `kthread_should_stop()`

list
~~~~

 - LIST_HEAD() дана функцію створює структуру `list_head`
 - Функцію `list_add_tail()` додає елемент в кінець

mutex
~~~~~

 - стрворює змінну за допомогою `DEFINE_MUTEX()`
 - `mutex_lock` блокуємо
 - `mutex_unlock` розблоковуємо

Результат
---------

module `mod3_no_sync`
~~~~~~~~~~~~~~

Без використання mutex

.. code-block::
	
	/mnt # insmod mod3_no_sync.ko thread_num=10 thread_inccnt=50 thread_delay=50
	[  174.400609] mod3_no_sync started with: thread_num = 10,
	[  174.400609]                   thread_inccnt = 50
	[  174.400609]                   thread_delay = 50
	.
	.
	.
	[  177.563403] mod3_no_sync: Thread glob_var: 494
	[  177.564442] mod3_no_sync: Thread glob_var: 495
	[  177.564507] mod3_no_sync: Thread glob_var: 493
	[  177.564844] mod3_no_sync: Thread glob_var: 499
	[  177.565173] mod3_no_sync: Thread glob_var: 497
	/mnt # rmmod mod3_no_sync.
	[  178.726873] mod3_no_sync: node[0] glob_var_saved = 498
	[  178.727086] mod3_no_sync: node[1] glob_var_saved = 496
	[  178.727500] mod3_no_sync: node[2] glob_var_saved = 492
	[  178.727616] mod3_no_sync: node[3] glob_var_saved = 494
	[  178.727880] mod3_no_sync: node[4] glob_var_saved = 491
	[  178.727998] mod3_no_sync: node[5] glob_var_saved = 493
	[  178.728156] mod3_no_sync: node[6] glob_var_saved = 499
	[  178.728269] mod3_no_sync: node[7] glob_var_saved = 490
	[  178.728381] mod3_no_sync: node[8] glob_var_saved = 495
	[  178.728482] mod3_no_sync: node[9] glob_var_saved = 497
	[  178.728947] mod3_no_sync: glob_var = 499	

Можемо бачити що результат не рівний 500, модуль відпрацював не вірно.

module `mod3_true_sync`
~~~~~~~~~~~~~~~~~~~~~

З використанням mutex

.. code-block::

   	/mnt # insmod mod3_true_sync.ko thread_num=10 thread_inccnt=100 thread_delay=1
	[  473.440283] mod3_true_sync started with: thread_num = 10,
	[  473.440283]                   thread_inccnt = 100
	[  473.440283]                   thread_delay = 1
	.
	.
	.
	[  436.244252] mod3_true_sync: Thread glob_var: 995
	[  436.245005] mod3_true_sync: Thread glob_var: 996
	[  436.279058] mod3_true_sync: Thread glob_var: 997
	[  436.280855] mod3_true_sync: Thread glob_var: 998
	[  436.282833] mod3_true_sync: Thread glob_var: 999
	[  436.285878] mod3_true_sync: Thread glob_var: 1000
	/mnt # rmmod mod3_true_sync.
	[  439.846076] mod3_true_sync: node[0] glob_var_saved = 935
	[  439.846531] mod3_true_sync: node[1] glob_var_saved = 951
	[  439.846844] mod3_true_sync: node[2] glob_var_saved = 958
	[  439.847086] mod3_true_sync: node[3] glob_var_saved = 981
	[  439.847262] mod3_true_sync: node[4] glob_var_saved = 986
	[  439.847380] mod3_true_sync: node[5] glob_var_saved = 985
	[  439.847699] mod3_true_sync: node[6] glob_var_saved = 993
	[  439.847957] mod3_true_sync: node[7] glob_var_saved = 995
	[  439.848116] mod3_true_sync: node[8] glob_var_saved = 994
	[  439.848237] mod3_true_sync: node[9] glob_var_saved = 1000
	[  439.848396] mod3_true_sync: glob_var = 1000

Результат рівний 1000, тобто модуль відпрацював вірно.

module `mod3_my_sync`
~~~~~~~~~~~~~~~~~~~~~

.. code-block::

	/mnt # insmod mod3_my_sync.ko thread_num=10 thread_inccnt=100 thread_delay=1
	[  539.582722] mod3_my_sync started with: thread_num = 10,
	[  539.582722]                   thread_inccnt = 100
	[  539.582722]                   thread_delay = 1
	.
	.
	.
	[  539.863702] mod3_my_sync: Thread glob_var: 996
	[  539.863782] mod3_my_sync: Thread glob_var: 997
	[  539.866730] mod3_my_sync: Thread glob_var: 999
	[  539.866780] mod3_my_sync: Thread glob_var: 998
	[  539.868746] mod3_my_sync: Thread glob_var: 1000

	/mnt # rmmod mod3_my_sync.
	[  572.338692] mod3_my_sync: node[0] glob_var_saved = 924
	[  572.339945] mod3_my_sync: node[1] glob_var_saved = 966
	[  572.340080] mod3_my_sync: node[2] glob_var_saved = 967
	[  572.340265] mod3_my_sync: node[3] glob_var_saved = 978
	[  572.340425] mod3_my_sync: node[4] glob_var_saved = 979
	[  572.340706] mod3_my_sync: node[5] glob_var_saved = 985
	[  572.340877] mod3_my_sync: node[6] glob_var_saved = 990
	[  572.340981] mod3_my_sync: node[7] glob_var_saved = 991
	[  572.341096] mod3_my_sync: node[8] glob_var_saved = 998
	[  572.341209] mod3_my_sync: node[9] glob_var_saved = 1000
        [  572.341363] mod3_my_sync: glob_var = 1000


Результат рівний 1000, модуль відпрацював вірно.

Висновок
--------
В даній роботі я розібрався зі списками, потоками та їхньої синхронізації в ядрі linux.
Реалізував модуль який перевіряє на коректність роботи при використанні `mutex-ів`.
