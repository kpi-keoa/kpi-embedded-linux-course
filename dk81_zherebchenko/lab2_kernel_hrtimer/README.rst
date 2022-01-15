==========================================================
**Лабораторна робота №2: Засоби відкладеної роботи: таймери та тасклети**
==========================================================

**Завдання:**
~~~~~~~~~~~~~

* Ознайомитися з механізмами вивільнення відкладеної роботи: таймерами та тасклетами, а також навчитися використовувати механізми динамічної алокації пам'яті в ядрі. 

**Хід роботи:**
~~~~~~~~~~~~~~~

Модуль ядра приймає два параметри: cnt (кількість циклів, які має відпрацювати таймер) та delay (затримка між двома спрацюваннями таймера).

.. code-block:: c

	static int cnt = NULL;
	static int delay = NULL;

	module_param(cnt, int, 0);
	MODULE_PARM_DESC(cnt, "Number of cycles to be worked by the timer");
	module_param(delay, int, 0);
	MODULE_PARM_DESC(delay, "Delay between two timer activations");

На початку init друкує поточне значення jiffies у лог ядра.

.. code-block:: c

	printk(KERN_INFO "[Init] Hello!\n[Init] jiffies = %lu\n", jiffies);

Потім запускається тасклет, який повинен надрукувати своє значення jiffies у лог ядра. 

.. code-block:: c

	tasklet_schedule(&first_tasklet);

Потім виділяється масив розміру cnt, використовуючи динамічну аллокацію kzalloc(). У цей масив таймер складатиме значення. 

.. code-block:: c

	arr = kzalloc(cnt * sizeof(*arr), GFP_KERNEL);

Потім init запускає таймер із затримкою delay та завершується функція.

.. code-block:: c

	hrtimer_init(&first_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	first_hrtimer.function = &timer_handler;
	hrtimer_start(&first_hrtimer, ms_to_ktime(delay), HRTIMER_MODE_REL);

При спрацюванні, таймер кладе поточне значення jiffies у масив та перезапускається із затримкою delay. Загальна кількість разів, які запускається таймер, дорівнює cnt.

В exit модуль друкує поточне значення jiffies, а також виводить всі значення з масиву. 

.. code-block:: c

	printk(KERN_INFO "[Exit] jiffies = %lu\n", jiffies);

	i = 0;
	while (i < t_cnt) {
		printk(KERN_INFO "[Exit] Сounter: %d, jiffies = %lu", i, arr[i]);
		i++;
	}

У випадку, якщо exit викликається до того, як таймер встигне відпрацювати cnt раз, скасовується виконання запланованого запуску таймера, виводиться в лог ядра повідомлення про дострокове вивантаження модуля і друкуються в лог ті елементи масиву, які встиг заповнити таймер.


**Отримані результати:**
~~~~~~~~~~~~~~~ 

Тест модуля ядра з передаванням параметрів cnt = 10, delay = 100 :

.. code-block:: bash

	/ # insmod /mnt/lab2_mod.ko cnt=10 delay=100
	[  120.046975] [Init] Hello!
	[  120.046975] [Init] jiffies = 4294787180
	[  120.053576] [Tasklet] jiffies = 4294787185
	/ # rmmod lab2_mod
	[  138.594384] [Exit] jiffies = 4294805728
	[  138.596947] [Exit] Counter: 0, jiffies = 4294787290
	[  138.596955] [Exit] Counter: 1, jiffies = 4294787390
	[  138.600070] [Exit] Counter: 2, jiffies = 4294787490
	[  138.609281] [Exit] Counter: 3, jiffies = 4294787590
	[  138.614173] [Exit] Counter: 4, jiffies = 4294787690
	[  138.616973] [Exit] Counter: 5, jiffies = 4294787790
	[  138.621545] [Exit] Counter: 6, jiffies = 4294787890
	[  138.633375] [Exit] Counter: 7, jiffies = 4294787990
	[  138.635330] [Exit] Counter: 8, jiffies = 4294788090
	[  138.636921] [Exit] Counter: 9, jiffies = 4294788190
	
Тест модуля ядра з передаванням параметра cnt = 0 :

.. code-block:: bash

	/ # insmod /mnt/lab2_mod.ko cnt=0 delay=100
	[  317.000757] [Init] Hello!
	[  317.000757] [Init] jiffies = 4294984134
	[  317.010146] [Init] Error, cnt must be greater than 0
	[  317.012554] [Tasklet] jiffies = 4294984146
	/ # rmmod lab2_mod
	[  344.879015] [Exit] jiffies = 4394990215

Тест модуля ядра з передаванням параметра delay = 0 :

.. code-block:: bash

	/ # insmod /mnt/lab2_mod.ko cnt=10 delay=0
	[   65.304968] lab2_mod: loading out-of-tree module taints kernel.
	[   65.309185] [Init] Hello!
	[   65.309185] [Init] jiffies = 4294732446
	[   65.312106] [Tasklet] jiffies = 4294732449
	[   65.314395] [Init] Error, delay must be greater than 0
	[   65.317214] insmod (98) used greatest stack depth: 13696 bytes left
	/ # rmmod lab2_mod
	[   83.146931] [Exit] jiffies = 4294738538

Тест модуля ядра з передаванням параметрів cnt = 10, delay = 2000, та достроковим вивантаженням модуля:

.. code-block:: bash

	/ # insmod /mnt/lab2_mod.ko cnt=10 delay=2000
	[  124.208737] [Init] Hello!
	[  124.208737] [Init] jiffies = 4294791344
	[  124.220381] [Tasklet] jiffies = 4294791355
	/ # rmmod lab2_mod
	[  130.261741] [Exit] jiffies = 4294797397
	[  130.263539] [Exit] Warning! Array isn't filled to end
	[  130.263543] [Exit] Counter: 0, jiffies = 4294793358
	[  130.265112] [Exit] Counter: 1, jiffies = 4294795358

Висновки
~~~~~~~~
При виконанні лабораторної роботи були вивчені механізми вивільнення відкладеної роботи: таймери та тасклети, а також використання механізмів динамічної алокації пам'яті в ядрі. Створено модуль ядра, в якому застосовані тасклети та hr таймери. Вдосконалені навички володіння мовою програмування C.
