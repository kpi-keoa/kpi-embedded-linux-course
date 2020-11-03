=====================
Лабораторна робота №4
=====================

Тема
------

**Потоки ядра та методи синхронізації**

Завдання:
---------

Написати та протестувати власний модуль ядра, який:
  * содержит глобальную переменную glob_var и параметры thread_num, thread_inccnt, thread_delay
  * запускает thread_num потоков на одновременное выполнение
  * каждый поток инкрементирует переменную glob_var thread_inccnt раз с задержкой thread_delay (может быть 0 — в этом случае, без задержки), кладет значение переменной в список и завершается
  * при выгрузке модуль выводит значение переменной glob_var и содержимое списка
  * для переменной, списка, потоков использовать динамическую аллокацию. Переменную передавать в поток аргументом
  * предусматривает возможность досрочной выгрузки и будет нормально отрабатывать в этом случае

також потрібно- продемонструвати результати, впровадити синхронізацію, прокоментувати зміни, реалізувати власну синхроніацію.

Хід роботи
----------

створений модуль ядра майже не включає в себе нічого з модулів які були створені в попередніх лабораторних роботах.Інформація для створення модуля була взята з дебільшого в лекціях а такох з рекомендованої літератури.
для забезпечення адекватного відпрацювання потоків було використано механізм completion йому було надано перевагу через те що завдяки йому можна написати простий для розуміння та відлагодження код, а також не переривати робоботу потоків при вигрузці модулю а очікувати завершення роботи всіх потків що забезпечить коректне відпрацювання модулю в переважній більшості ситуацій.
так як критичні ділянки коду знаходяться не в атомарному контексті то можна використати mutex для синхронізації, його і використаємо тому що spinlock набагато сильніше навантажує систему.
свою реалізацію синхронізації було створено на основі операції атомарного обміну.



Результати
----------

перший рядок параметри виклику insmod подалші рядки відпрацювання rmmod

без синхронізації:

.. code-block:: bash

	insmod mnt/mymod.ko thr_incrcnt=100000 thr_cnt=10 thr_delay=0
	[   30.793702] count_val = 226327
	[   30.793800] count_val = 276844
	[   30.794735] count_val = 278859
	[   30.795623] count_val = 378859
	[   30.796532] count_val = 508093
	[   30.797470] count_val = 513929
	[   30.798329] count_val = 603436
	[   30.799267] count_val = 642912
	[   30.799977] count_val = 742912
	[   30.800817] count_val = 842912
	[   30.801951] stoping the thread named: thread_0 	 jiffies 4294698122
	[   30.805139] stoping the thread named: thread_1 	 jiffies 4294698126
	[   30.806807] stoping the thread named: thread_2 	 jiffies 4294698127
	[   30.807684] stoping the thread named: thread_3 	 jiffies 4294698127
	[   30.809513] stoping the thread named: thread_4 	 jiffies 4294698130
	[   30.811199] stoping the thread named: thread_5 	 jiffies 4294698132
	[   30.812569] stoping the thread named: thread_6 	 jiffies 4294698133
	[   30.813513] stoping the thread named: thread_7 	 jiffies 4294698134
	[   30.815378] stoping the thread named: thread_8 	 jiffies 4294698136
	[   30.817183] stoping the thread named: thread_9 	 jiffies 4294698138



mutex:

.. code-block:: bash

	insmod mnt/mymod.ko thr_incrcnt=100000 thr_cnt=10 thr_delay=0
 	[  225.080961] count_val = 100000
	[  225.082116] count_val = 200000
	[  225.082669] count_val = 300000
	[  225.082924] count_val = 400000
	[  225.083351] count_val = 500000
	[  225.083711] count_val = 600000
	[  225.084235] count_val = 700000
	[  225.084580] count_val = 800000
	[  225.084944] count_val = 900000
	[  225.085186] count_val = 1000000
	[  380.693158] stoping the thread named: thread_0 	 jiffies 4295048020
	[  380.694730] stoping the thread named: thread_1 	 jiffies 4295048021
	[  380.695651] stoping the thread named: thread_2 	 jiffies 4295048022
	[  380.696657] stoping the thread named: thread_3 	 jiffies 4295048023
	[  380.697682] stoping the thread named: thread_4 	 jiffies 4295048024
	[  380.698451] stoping the thread named: thread_5 	 jiffies 4295048025
	[  380.699141] stoping the thread named: thread_6 	 jiffies 4295048025
	[  380.700254] stoping the thread named: thread_7 	 jiffies 4295048027
	[  380.701110] stoping the thread named: thread_8 	 jiffies 4295048027
	[  380.701858] stoping the thread named: thread_9 	 jiffies 4295048028



власна:

.. code-block:: bash
	insmod mnt/mymod.ko thr_incrcnt=100000 thr_cnt=10 thr_delay=0
	[  225.080961] count_val = 100000
	[  225.082116] count_val = 200000
	[  225.082669] count_val = 300000
	[  225.082924] count_val = 400000
	[  225.083351] count_val = 500000
	[  225.083711] count_val = 600000
	[  225.084235] count_val = 700000
	[  225.084580] count_val = 800000
	[  225.084944] count_val = 900000
	[  225.085186] count_val = 1000000
	[  380.693158] stoping the thread named: thread_0 	 jiffies 4295048020
	[  380.694730] stoping the thread named: thread_1 	 jiffies 4295048021
	[  380.695651] stoping the thread named: thread_2 	 jiffies 4295048022
	[  380.696657] stoping the thread named: thread_3 	 jiffies 4295048023
	[  380.697682] stoping the thread named: thread_4 	 jiffies 4295048024
	[  380.698451] stoping the thread named: thread_5 	 jiffies 4295048025
	[  380.699141] stoping the thread named: thread_6 	 jiffies 4295048025
	[  380.700254] stoping the thread named: thread_7 	 jiffies 4295048027
	[  380.701110] stoping the thread named: thread_8 	 jiffies 4295048027
	[  380.701858] stoping the thread named: thread_9 	 jiffies 4295048028


Висновки
--------
в ході виконання роботи було створено модуль ядра для демонстрації роботи потоків
списків та синхронізації. Загалом, зпочатку було створено модуль без синжронізації
як видно з результатів відпрацювання модуля для коректної роботи модуля потрібно
додатити синхронізацію до критичних частин коду, пілся додання синхронізації
було отримано очікувані результати інкремент змінної відбувався корректно.
