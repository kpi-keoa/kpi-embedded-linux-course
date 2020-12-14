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

В кінцевому результаті необхідно продемонструвати результати, додати синхронізацію, прокоментувати зміни, реалізувати власну синхроніацію.

Хід роботи:
----------

При виконанні декількох потоків вони будуть незмінно взаємодіяти один з одним, щоб синхронізувати своє виконання.
Існує кілька методів синхронізації потоків:
  * Блоки взаємного виключення (Mutex);
  * Спінлоки;
  * Семафори.
Завдяки об'єктам синхронізації багатопоточна програма може без проблем опрацьовувати критичні ділянки коду та дозволяє
працювати потокам і при конкурентній роботі не руйнючи RMW послідовність одне одного.

Для створення даного модуля ядра майже не використовувалися частини модулів, які були створені в попередніх лабораторних роботах.
Інформація для створення модуля була взята з дебільшого в лекціях, а також з рекомендованої літератури. Для забезпечення адекватного
відпрацювання потоків було використано механізм completion йому було надано перевагу через те, що завдяки йому можна написати простий
для розуміння та відлагодження код, а також не переривати робоботу потоків при вигрузці модуля, а очікувати завершення роботи всіх потків,
що забезпечить коректне відпрацювання модулю в переважній більшості ситуацій. Так як критичні ділянки коду знаходяться не в атомарному контексті,
то можна використати mutex для синхронізації, його і використаємо тому що spinlock набагато сильніше навантажує систему.
Свою реалізацію синхронізації було створено на основі операції атомарного обміну.


Результати:
----------

Далі зображено відпрацювання ініт та ексіт функції.

Без синхронізації:

.. code-block:: bash

	/ # insmod mnt/no_sync.ko thr_incrcnt=100000 thr_cnt=10 thr_delay=0
	[   98.759565] no_sync: loading out-of-tree module taints kernel.
	[   98.794327] I am 0-rd
	[   98.798244] thread params : ammount of jiffies - 0 ; increment - 100000
	[   98.802842] I am 1-rd
	[   98.810349] thread params : ammount of jiffies - 0 ; increment - 100000
	[   98.816506] I am 3-rd
	[   98.816547] I am 2-rd
	[   98.816605] thread params : ammount of jiffies - 0 ; increment - 100000
	[   98.819622] thread params : ammount of jiffies - 0 ; increment - 100000
	[   98.825368] I am 4-rd
	[   98.826225] thread params : ammount of jiffies - 0 ; increment - 100000
	[   98.835595] I am 6-rd
	[   98.837563] I am 5-rd
	[   98.837918] thread params : ammount of jiffies - 0 ; increment - 100000
	[   98.838345] thread params : ammount of jiffies - 0 ; increment - 100000
	[   98.842228] I am 7-rd
	[   98.847727] thread params : ammount of jiffies - 0 ; increment - 100000
	[   98.849142] I am 8-rd
	[   98.851957] thread params : ammount of jiffies - 0 ; increment - 100000
	[   98.854945] I am 9-rd
	[   98.858954] thread params : ammount of jiffies - 0 ; increment - 100000
	[   98.867493] insmod (97) used greatest stack depth: 13696 bytes left
	/ # rmmod no_sync.ko
	[  130.620595] thread number 0 has 100000 count_val
	[  130.620696] thread number 1 has 200000 count_val
	[  130.621015] thread number 2 has 300127 count_val
	[  130.622030] thread number 3 has 369196 count_val
	[  130.622512] thread number 4 has 469196 count_val
	[  130.622986] thread number 6 has 569196 count_val
	[  130.623675] thread number 5 has 704102 count_val
	[  130.624264] thread number 7 has 737308 count_val
	[  130.624923] thread number 8 has 791588 count_val
	[  130.625480] thread number 9 has 891588 count_val
	[  130.626059] for the kernel !!!
	[  130.626786] stoping the thread jiffies 4294797950
	[  130.627878] stoping the thread jiffies 4294797952
	[  130.628546] stoping the thread jiffies 4294797952
	[  130.629122] stoping the thread jiffies 4294797953
	[  130.629744] stoping the thread jiffies 4294797953
	[  130.630460] stoping the thread jiffies 4294797954
	[  130.631098] stoping the thread jiffies 4294797954
	[  130.631815] stoping the thread jiffies 4294797956
	[  130.632389] stoping the thread jiffies 4294797956
	[  130.632770] stoping the thread jiffies 4294797956


Mutex:

.. code-block:: bash

	/ # insmod mnt/mutex_sync.ko thr_incrcnt=100000 thr_cnt=10 thr_delay=0
	[  110.719973] I am 0-rd
	[  110.720420] thread params : ammount of jiffies - 0 ; increment - 100000
	[  110.721988] I am 1-rd
	[  110.722729] thread params : ammount of jiffies - 0 ; increment - 100000
	[  110.735223] I am 2-rd
	[  110.739154] thread params : ammount of jiffies - 0 ; increment - 100000
	[  110.748967] I am 3-rd
	[  110.750945] thread params : ammount of jiffies - 0 ; increment - 100000
	[  110.757320] I am 4-rd
	[  110.766854] thread params : ammount of jiffies - 0 ; increment - 100000
	[  110.780989] I am 5-rd
	[  110.781860] thread params : ammount of jiffies - 0 ; increment - 100000
	[  110.789098] I am 6-rd
	[  110.791418] thread params : ammount of jiffies - 0 ; increment - 100000
	[  110.805941] I am 7-rd
	[  110.807848] thread params : ammount of jiffies - 0 ; increment - 100000
	[  110.808713] I am 8-rd
	[  110.810053] thread params : ammount of jiffies - 0 ; increment - 100000
	[  110.827977] I am 9-rd
	/ # rmmod mutex_sync
	[  110.837877] thread params : ammount of jiffies - 0 ; increment - 100000
	[  133.328499] thread number 4 has 607655 count_val
	[  133.329822] thread number 1 has 623946 count_val
	[  133.330367] thread number 8 has 682987 count_val
	[  133.330586] thread number 0 has 683051 count_val
	[  133.331662] thread number 5 has 830310 count_val
	[  133.332228] thread number 7 has 937452 count_val
	[  133.332604] thread number 6 has 951526 count_val
	[  133.333669] thread number 3 has 979426 count_val
	[  133.334694] thread number 9 has 991978 count_val
	[  133.335360] thread number 2 has 1000000 count_val
	[  133.336089] for the kernel !!!
	[  133.336531] stoping the thread jiffies 4294800661
	[  133.337586] stoping the thread jiffies 4294800663
	[  133.338107] stoping the thread jiffies 4294800663
	[  133.338680] stoping the thread jiffies 4294800664
	[  133.339633] stoping the thread jiffies 4294800664
	[  133.340384] stoping the thread jiffies 4294800665
	[  133.341268] stoping the thread jiffies 4294800666
	[  133.341890] stoping the thread jiffies 4294800666
	[  133.342444] stoping the thread jiffies 4294800667
	[  133.343131] stoping the thread jiffies 4294800668


Власна:

.. code-block:: bash

	/ # insmod mnt/my_spin_sync.ko thr_incrcnt=100000 thr_cnt=10 thr_delay=0
	[   80.480147] my_spin_sync: loading out-of-tree module taints kernel.
	[   80.513598] I am 0-rd
	[   80.515014] thread params : ammount of jiffies - 0 ; increment - 100000
	[   80.522295] I am 1-rd
	[   80.522405] I am 2-rd
	[   80.525380] thread params : ammount of jiffies - 0 ; increment - 100000
	[   80.526332] thread params : ammount of jiffies - 0 ; increment - 100000
	[   80.535704] I am 3-rd
	[   80.536473] thread params : ammount of jiffies - 0 ; increment - 100000
	[   80.537247] I am 4-rd
	[   80.539438] thread params : ammount of jiffies - 0 ; increment - 100000
	[   80.595761] I am 5-rd
	[   80.605322] thread params : ammount of jiffies - 0 ; increment - 100000
	[   80.606616] I am 6-rd
	[   80.607742] I am 7-rd
	[   80.607927] thread params : ammount of jiffies - 0 ; increment - 100000
	[   80.608829] thread params : ammount of jiffies - 0 ; increment - 100000
	[   80.612638] I am 8-rd
	[   80.614042] thread params : ammount of jiffies - 0 ; increment - 100000
	[   80.686147] I am 9-rd
	[   80.688645] insmod (98) used greatest stack depth: 13696 bytes left
	/ # lsmod 
	my_spin_sync 16384 0 - Live 0xffffffffc0392000 (O)
	/ # rmmod ^C
	/ # rmmod my_spin_sync
	[   80.690303] thread params : ammount of jiffies - 0 ; increment - 100000
	[  129.010501] thread number 0 has 141214 count_val
	[  129.011733] thread number 3 has 470872 count_val
	[  129.012575] thread number 1 has 478469 count_val
	[  129.013459] thread number 2 has 491014 count_val
	[  129.014273] thread number 4 has 500000 count_val
	[  129.014810] thread number 5 has 843207 count_val
	[  129.015226] thread number 7 has 875456 count_val
	[  129.015939] thread number 8 has 882655 count_val
	[  129.016472] thread number 6 has 900000 count_val
	[  129.017422] thread number 9 has 1000000 count_val
	[  129.018126] for the kernel !!!
	[  129.018679] stoping the thread jiffies 4294796344
	[  129.019657] stoping the thread jiffies 4294796345
	[  129.020210] stoping the thread jiffies 4294796345
	[  129.021099] stoping the thread jiffies 4294796347
	[  129.021657] stoping the thread jiffies 4294796347
	[  129.022051] stoping the thread jiffies 4294796347
	[  129.022808] stoping the thread jiffies 4294796348
	[  129.023327] stoping the thread jiffies 4294796348
	[  129.023932] stoping the thread jiffies 4294796349
	[  129.024394] stoping the thread jiffies 4294796349


Висновки:
--------
При виконанні даної лабораторної роботи було створено модуль ядра для демонстрації роботи потоків
списків та синхронізації. Загалом, спочатку було створено модуль без синжронізації.
Як видно з результатів відпрацювання модуля для коректної роботи модуля потрібно
додатити синхронізацію до критичних частин коду, пілся додання синхронізації
було отримано очікувані результати. Інкремент змінної відбувався корректно.
