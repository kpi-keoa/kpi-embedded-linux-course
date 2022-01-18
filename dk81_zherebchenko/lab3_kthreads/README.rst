==========================================================
**Лабораторна робота №3: Kernel Threads**
==========================================================

**Завдання:**
~~~~~~~~~~~~~

* Навчитися працювати з потоками ядра.
* Ознайомитись зі зв'язними списками ядра.
* Навчитися використовувати механізми синхронізації в ядрі.
* Створити відповідний модуль в якому йде робота з потоками, списками, та механізмами синхронізації в ядрі.

**Хід роботи:**
~~~~~~~~~~~~~~~

Модуль ядра містить глобальну змінну glob_var та параметри thread_num, thread_inccnt, thread_delay :

.. code-block:: c

	static volatile unsigned long glob_var = 0;
	static unsigned int thread_num;
	static unsigned int thread_inccnt;
	static unsigned int thread_delay;
	
	module_param(thread_num, uint, 0);
	MODULE_PARM_DESC(thread_num, "Number of threads for simultaneous execution");
	module_param(thread_inccnt, uint, 0);
	MODULE_PARM_DESC(thread_inccnt, "Number of times each thread increments the "
					"glob_var variable");
	module_param(thread_delay, uint, 0);
	MODULE_PARM_DESC(thread_delay, "Delay between increments of the glob_var "
					"variable by each thread");
	
thread_num - кількість потоків для одночасного виконання.

thread_inccnt - кількість разів, коли кожний потік збільшує глобальну змінну glob_var.

thread_delay - затримка між інкрементаціями глобальної змінної glob_var кожним потоком.

Тобто, кожен потік інкрементує змінну glob_var thread_inccnt раз із затримкою thread_delay, потім кладе значення змінної до списку і завершується.

.. code-block:: c

	static int thread_func(void *var)
	{
		int j = 0;
		struct v_list *n = kzalloc(sizeof(*n), GFP_KERNEL);

		while (j < thread_inccnt) {
			spin_lock(&lock);
			(*(typeof(glob_var)*)var)++;
			spin_unlock(&lock);
		
			if (thread_delay != 0) {
				schedule_timeout_uninterruptible
				(msecs_to_jiffies(thread_delay));
                	}
			if (kthread_should_stop()) {
				spin_lock(&lock);
				break;
			}
                	j++;
		}
		if (j >= thread_inccnt) {
			spin_lock(&lock);
        	}
        
		n -> cnt_val = (*(typeof(glob_var)*)var);
		list_add_tail(&(n -> head), &list);
		spin_unlock(&lock);
		return 0;
	}

Для змінної списку та потоків використовується динамічна аллокація kzalloc() :

.. code-block:: c

	struct v_list *n = kzalloc(sizeof(*n), GFP_KERNEL);

	thread_dat = kzalloc(thread_num * sizeof(*thread_dat), GFP_KERNEL);

В exit модуль виводить спочатку значення глобальної змінної glob_var, а потім вміст списку:

.. code-block:: c

	printk(KERN_INFO "[Exit] Final glob_var = %ld\n", glob_var);

	list_for_each_safe(tmp, next, &list) {
		struct v_list *cur = list_entry(tmp, struct v_list, head);
		printk(KERN_INFO "[Exit] glob_var = %ld\n", cur -> cnt_val);
		list_del(tmp);
		kfree(cur);
	}

**Отримані результати:**
~~~~~~~~~~~~~~~

Тест модуля ядра з синхронізацією використовуючи SPINLOCK:

.. code-block:: bash

	/ # insmod mnt/lab3_sync.ko thread_num=10 thread_inccnt=10 thread_delay=10
	[  138.495461] [Init] Hello!
	[  138.495461] [Init] jiffies = 4294805628
	/ # rmmod lab3_sync
	[  139.956243] [Exit] Final glob_var = 100
	[  139.958066] [Exit] glob_var = 100
	[  139.959543] [Exit] glob_var = 100
	[  139.961063] [Exit] glob_var = 100
	[  139.963541] [Exit] glob_var = 100
	[  139.965108] [Exit] glob_var = 100
	[  139.966626] [Exit] glob_var = 100
	[  139.968159] [Exit] glob_var = 100
	[  139.971828] [Exit] glob_var = 100
	[  139.974242] [Exit] glob_var = 100
	[  139.975785] [Exit] glob_var = 100

Тест модуля ядра, якщо не використовувати механізми синхронізації :

.. code-block:: bash

	/ # insmod mnt/lab3_no_sync.ko thread_num=10 thread_inccnt=10 thread_delay=1000
	[   72.090283] lab3_no_sync: loading out-of-tree module taints kernel.
	[   72.095819] [Init] Hello!
	[   72.095819] [Init] jiffies = 4294739228
	[   72.108325] insmod (100) used greatest stack depth: 13696 bytes left
	/ # rmmod lab3_no_sync
	[   84.981982] [Exit] Final glob_var = 100
	[   84.989188] [Exit] glob_var = 91
	[   84.992613] [Exit] glob_var = 92
	[   84.994103] [Exit] glob_var = 93
	[   84.995716] [Exit] glob_var = 94
	[   84.997242] [Exit] glob_var = 95
	[   84.998754] [Exit] glob_var = 96
	[   85.000290] [Exit] glob_var = 97
	[   85.001831] [Exit] glob_var = 98
	[   85.003329] [Exit] glob_var = 99
	[   85.004841] [Exit] glob_var = 100

Тест модуля ядра використовуючи власні реалізації функцій lock() та unlock() для синхронізації:

.. code-block:: bash

	/ # insmod mnt/lab3_my_sync.ko thread_num=10 thread_inccnt=10 thread_delay=10
	[   71.093910] [Init] Hello!
	[   71.093910] [Init] jiffies = 4294738231
	/ # rmmod lab3_my_sync
	[   72.709041] [Exit] Final glob_var = 100
	[   72.710802] [Exit] glob_var = 100
	[   72.712626] [Exit] glob_var = 100
	[   72.714100] [Exit] glob_var = 100
	[   72.715566] [Exit] glob_var = 100
	[   72.717020] [Exit] glob_var = 100
	[   72.718508] [Exit] glob_var = 100
	[   72.720917] [Exit] glob_var = 100
	[   72.722394] [Exit] glob_var = 100
	[   72.723872] [Exit] glob_var = 100
	[   72.725356] [Exit] glob_var = 100

Висновки
~~~~~~~~

При виконанні лабораторної роботи був створений модуль, в якому йшла робота з потоками ядра, застосуванням списків та механізмів синхронізації потоків у ядрі. Також був створений ще один модуль, який використовує власні реалізації lock() і unlock() з використанням атомарних операцій ядра. Вдосконалені навички володіння мовою програмування C.

