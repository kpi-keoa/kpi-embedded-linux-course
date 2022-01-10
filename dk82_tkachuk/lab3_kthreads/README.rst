=======================================
**Лабораторна робота №3 "Потоки ядра"**
=======================================

**Завдання:**
~~~~~~~~~~~~~
Написати модуль ядра, який:
    * Містить глобальну змінну *glob_var* та параметри *thread_num*, *thread_inccnt*, *thread_delay*.
	* Запускає thread_num потоків на одночасне виконання.
	* Кожен потік інкрементує змінну *glob_var* *thread_inccnt* раз із затримкою *thread_delay* (може бути 0 - у цьому випадку, без затримки), кладе значення змінної до списку та завершується.
	* При вивантаженні модуль виводить значення змінної glob_var та вміст списку.
	* Для змінної, списку, потоків використовувати динамічну аллокацію. Змінну передавати до потоку аргументом.
	* Передбачити можливість дострокового відвантаження і нормальне відпрацьовування в цьому випадку.
	* Зробити другий варіант коду модуля, який використав власні реалізації *lock()* і *unlock()* з використанням атомарних операцій ядра (atomic.h, відрізняється в залежності від архітектури). Продемонструйте працездатність. Можна використовувати функцію *xchg*.

Продемонструвати, що без синхронізації підсумкове значення глобальної змінної *glob_var* неправильне.
Захистіть збільшення лічильника, а також доступ до списку (та інших шарених елементів) механізмами блокування та продемонструйте, що кількість відліків правильна.

**Хід роботи:**
~~~~~~~~~~~~~~~

	* Створюємо список *glob_var_list*, масив потоків *thread* та структуру *list_node*, яка наслідує структуру *list_head* та зберігає значення змінної glob_var.
    * Створюємо функцію *thread_func*, яка виконуватиметься у потоці.
	* У функції *__init thirdmod_init* перевіряємо правильність значень параметрів модуля. Перевіряємо успішність виділення пам'яті для масиву *thread*. Далі відбувається запуск кожного з потоків.
    * У функції __exit thirdmod_exit відповідно зупиняються потоки, вивільняється пам'ять масива та виводиться вміст списку та значення glob_var.

**Результат:**
~~~~~~~~~~~~~~

.. code-block:: bash

	Запуск без використання mutex. За замовчуванням thread_num = 10; thread_inccnt = 200; thread_delay = 10.

	/ # insmod /mnt/mod_for_lab3_no_lock.ko
    [   81.339198] Modue mod_for_lab3_no_lock is installed!
    [   81.339378] init glob_var = 0
    [   81.340201] thread-0 was created suceccfuly!
    [   81.341226] thread-1 was created suceccfuly!
    [   81.342210] thread-2 was created suceccfuly!
    [   81.342992] thread-3 was created suceccfuly!
    [   81.343475] thread-4 was created suceccfuly!
    [   81.344289] thread-5 was created suceccfuly!
    [   81.344988] thread-6 was created suceccfuly!
    [   81.345502] thread-7 was created suceccfuly!
    [   81.346219] thread-8 was created suceccfuly!
    [   81.347222] thread-9 was created suceccfuly!
    / # rmmod mod_for_lab3_no_lock
    [   84.951842] thread-0 was stoped
    [   84.952303] thread-1 was stoped
    [   84.952403] thread-2 was stoped
    [   84.952480] thread-3 was stoped
    [   84.952567] thread-4 was stoped
    [   84.952640] thread-5 was stoped
    [   84.952884] thread-6 was stoped
    [   84.953061] thread-7 was stoped
    [   84.953156] thread-8 was stoped
    [   84.953238] thread-9 was stoped
    [   84.953361] final glob_var = 1999
    [   84.953448] glob_var = 1986
    [   84.953548] glob_var = 1990
    [   84.953616] glob_var = 1993
    [   84.953783] glob_var = 1992
    [   84.953901] glob_var = 1994
    [   84.954056] glob_var = 1996
    [   84.954118] glob_var = 1997
    [   84.954176] glob_var = 1995
    [   84.954236] glob_var = 1999
    [   84.954307] glob_var = 1998
    [   84.954401] Modue mod_for_lab3_no_lock is removed
    / #

    Результат не збігається з очікуваним, який має бути 2000, натомість отримали результат 1998.

	Результат роботи модуля з синхронізацією, затримка рівна нулю:

	/ # insmod /mnt/mod_for_lab3_with_lock.ko thread_num=10 thread_inccnt=200 thread_delay=0
    [  488.970329] Modue mod_for_lab3_with_lock is installed!
    [  488.970537] init glob_var = 0
    [  488.971361] thread-0 was created suceccfuly!
    [  488.972284] thread-1 was created suceccfuly!
    [  488.973118] thread-2 was created suceccfuly!
    [  488.973848] thread-3 was created suceccfuly!
    [  488.974354] thread-4 was created suceccfuly!
    [  488.975081] thread-5 was created suceccfuly!
    [  488.975558] thread-6 was created suceccfuly!
    [  488.976328] thread-7 was created suceccfuly!
    [  488.977073] thread-8 was created suceccfuly!
    [  488.977667] thread-9 was created suceccfuly!
    / # rmmod mod_for_lab3_with_lock
    [  497.475215] thread-0 was stoped
    [  497.475386] thread-1 was stoped
    [  497.475475] thread-2 was stoped
    [  497.475549] thread-3 was stoped
    [  497.475619] thread-4 was stoped
    [  497.475920] thread-5 was stoped
    [  497.475993] thread-6 was stoped
    [  497.476070] thread-7 was stoped
    [  497.476145] thread-8 was stoped
    [  497.476232] thread-9 was stoped
    [  497.476339] final glob_var = 2000
    [  497.476436] glob_var = 1753
    [  497.476515] glob_var = 1755
    [  497.476580] glob_var = 1766
    [  497.476641] glob_var = 1770
    [  497.476969] glob_var = 1772
    [  497.477103] glob_var = 1773
    [  497.477311] glob_var = 1775
    [  497.477407] glob_var = 1965
    [  497.477490] glob_var = 1972
    [  497.477582] glob_var = 2000
    [  497.477816] Modue mod_for_lab3_with_lock is removed
    / #

    Результат збігається з очікуваним.

    Другий варіант модуля, який використовує власні реалізації *lock()* и *unlock()*

	/ # insmod /mnt/mod_for_lab3_my_lock.ko thread_num=10 thread_inccnt=200 thread_delay=0
    [  854.873331] Modue mod_for_lab3_my_lock is installed!
    [  854.873539] init glob_var = 0
    [  854.874451] thread-0 was created suceccfuly!
    [  854.875211] thread-1 was created suceccfuly!
    [  854.876063] thread-2 was created suceccfuly!
    [  854.876571] thread-3 was created suceccfuly!
    [  854.877143] thread-4 was created suceccfuly!
    [  854.877913] thread-5 was created suceccfuly!
    [  854.884334] thread-6 was created suceccfuly!
    [  854.890713] thread-7 was created suceccfuly!
    [  854.903810] thread-8 was created suceccfuly!
    [  854.989948] thread-9 was created suceccfuly!
    / # rmmod mod_for_lab3_my_lock
    [  862.611491] thread-0 was stoped
    [  862.612480] thread-1 was stoped
    [  862.612843] thread-2 was stoped
    [  862.613055] thread-3 was stoped
    [  862.613136] thread-4 was stoped
    [  862.613224] thread-5 was stoped
    [  862.613303] thread-6 was stoped
    [  862.613405] thread-7 was stoped
    [  862.613482] thread-8 was stoped
    [  862.613559] thread-9 was stoped
    [  862.613847] final glob_var = 2000
    [  862.614042] glob_var = 1604
    [  862.614153] glob_var = 1610
    [  862.614217] glob_var = 1755
    [  862.614287] glob_var = 1790
    [  862.614355] glob_var = 1850
    [  862.614427] glob_var = 1873
    [  862.614487] glob_var = 1891
    [  862.614551] glob_var = 1917
    [  862.614633] glob_var = 1963
    [  862.614872] glob_var = 2000
    [  862.615018] Modue mod_for_lab3_my_lock is removed
    / #

    Результат збігається з очікуваним.
**Висновок:**
~~~~~~~~~~~~~~~~~~~~~~~~~~~

	* На даній лабораторній роботі був розроблений модуль, який створює thread_num потоків, кожен з яких інкрементує значення змінної glob_var thread_inccnt разів з затрімкою в thread_delay. Модуль працює не коректно при відсутності синхронізації. Із власними реалізаціями *lock()*, *unlock()* модуль працює коректно.
