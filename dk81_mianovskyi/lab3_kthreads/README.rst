=====================================
**Лабораторна робота №3 Потоки ядра**
=====================================

Завдання:
---------------
Написати модуль ядра, який виконує наступні дії:

* Приймає три параметри thread_num, thread_inccnt та thread_delay
* Запускає thread_num потоків на одночасне виконання
* Кожен поток інкрементує глобальну змінну glob_var thread_inccnt разів із затримкою thread_delay
* Зберігає отримане значення змінної у список
* При вивантаженні модуль повинен виводити остаточне значення glob_var та вміст списку

**Хід роботи**

Спершу було створено список glob_var_list, масив потоків thread та структуру list_node, яка наслідує структуру list_head та зберігатиме значення змінної glob_var:

.. code-block:: bash

 static LIST_HEAD(glob_var_list);
 
 static struct task_struct **thread = NULL;
 
 struct list_node {
 	struct list_head list;
 	unsigned long glob_var_val;
 };

Далі було створено функцію, яка виконуватиметься у потоці:

.. code-block:: bash

 static int thread_func(void *data)
 {
 	struct list_node *n = kmalloc(sizeof(*n), GFP_KERNEL);
 	int cnt = thread_inccnt;	
 
 	INIT_LIST_HEAD(&n->list);
 	while(!kthread_should_stop() && cnt--){
 		n->glob_var_val = ++(*(volatile int *)data);
 		schedule_timeout_uninterruptible(msecs_to_jiffies(thread_delay));
  	}
 	
 	list_add_tail(&(n->list), &glob_var_list);
 	
 	return 0;
 }

У функції __init thirdmod_init було написано блок перевірки правильності значень уведених параметрів модуля, та перевірка на успішність виділення пам'яті під масив thread:

.. code-block:: bash

	int i;

        pr_info("Modue %s is installed!\n", module_name(THIS_MODULE));
	pr_info("init glob_var = %lu\n", glob_var);

	if(thread_num <= 0){
	pr_err("Error: invalid number of threads!\n");
	goto init_final;
	}
	if(thread_inccnt <= 0 ){
	pr_err("Error: invalid number of iterations!\n");
	goto init_final;
	}
	if(thread_delay < 0 ){
	pr_err("Error: invalid delay value!\n");
	goto init_final;
	}

	thread = kmalloc(thread_num*sizeof(**thread), GFP_KERNEL);
	if(NULL == thread){
	pr_err("Error: cannot allocate memory for threads!\n");
	status = -ENOMEM;
	goto init_final;
	}

	...

 init_final:
        return status;

Далі відбувається запуск кожного з потоків:

.. code-block:: bash
 
	for (i = 0; i < thread_num; i++){
		thread[i] = kthread_run(thread_func, &glob_var, "thread-%d", i);

		if(IS_ERR(thread[i])){
			pr_err("Error: thread-%d was not created!\n", i);
			thread[i] = NULL;
		}else{
			pr_info("thread-%d was created suceccfuly!\n", i);
		}
	get_task_struct(thread[i]);
	}

У функції __exit thirdmod_exit відповідно зупиняються потоки, вивільняється пам'ять масива та виводиться вміст списку та значення glob_var:

.. code-block:: bash

 int i;
	struct list_node *pos = NULL;
	struct list_node *n;
	
	if(NULL == thread)
		goto exit_final;

	for (i = 0; i < thread_num; i++)
	{
        	if(NULL == thread[i])
			goto skip;

		kthread_stop(thread[i]);
		put_task_struct(thread[i]);
 skip:
		pr_info("thread-%d was stoped\n", i);
	}
	kfree(thread);

	list_for_each_entry_safe(pos, n, &glob_var_list, list)
	{
		pr_info("glob_var = %lu\n", pos->glob_var_val);
		list_del(&(pos->list));
		kfree(pos);
	}
	
	pr_info("final glob_var = %lu\n", glob_var);

 exit_final:
	pr_info("Modue %s is removed\n", module_name(THIS_MODULE));

**Результат роботи створеного модуля**

Результат роботи модуля, без уведених аргументів:

.. code-block:: bash

 / # insmod /mnt/no_synch.ko
 [   17.752609] no_synch: loading out-of-tree module taints kernel.
 [   17.763959] Modue no_synch is installed!
 [   17.763959] init glob_var = 0
 [   17.767148] thread-0 was created suceccfuly!
 [   17.769895] insmod (97) used greatest stack depth: 13672 bytes left
 / # rmmod no_synch
 [   28.925796] thread-0 was stoped
 [   28.926131] final glob_var = 10
 [   28.926451] glob_var = 10
 [   28.926604] Modue no_synch is removed
 / # 

За замовчуванням, кількість потоків рівна 1, кількість інкрементацій 10 і затримка рівна 1 мілісекунді.

Результат роботи модуля без синхронізації з уведеними thread_num=10, thread_inccnt=500, thread_delay=0:

.. code-block:: bash

 / # insmod /mnt/no_synch.ko thread_num=10 thread_inccnt=500 thread_delay=0
 [   81.314707] Modue no_synch is installed!
 [   81.315062] init glob_var = 0
 [   81.316494] thread-0 was created suceccfuly!
 [   81.318119] thread-1 was created suceccfuly!
 [   81.319255] thread-2 was created suceccfuly!
 [   81.320172] thread-3 was created suceccfuly!
 [   81.320819] thread-4 was created suceccfuly!
 [   81.322250] thread-5 was created suceccfuly!
 [   81.322894] thread-6 was created suceccfuly!
 [   81.323677] thread-7 was created suceccfuly!
 [   81.335630] thread-8 was created suceccfuly!
 [   81.336654] thread-9 was created suceccfuly!
 / # rmmod no_synch
 [   92.298364] thread-0 was stoped
 [   92.298752] thread-1 was stoped
 [   92.299554] thread-2 was stoped
 [   92.300037] thread-3 was stoped
 [   92.300671] thread-4 was stoped
 [   92.301558] thread-5 was stoped
 [   92.302092] thread-6 was stoped
 [   92.302491] thread-7 was stoped
 [   92.303398] thread-8 was stoped
 [   92.303785] thread-9 was stoped
 [   92.304283] final glob_var = 4991
 [   92.304596] glob_var = 4502
 [   92.304706] glob_var = 4599
 [   92.304850] glob_var = 4824
 [   92.305081] glob_var = 4869
 [   92.305165] glob_var = 4875
 [   92.305362] glob_var = 4938
 [   92.305520] glob_var = 4949
 [   92.305691] glob_var = 4977
 [   92.305850] glob_var = 4989
 [   92.306086] glob_var = 4991
 [   92.306308] Modue no_synch is removed
 / # 

Як видно, результат не збігається з очікуваним, який має бути 5000, натомість отримали результат 4991.

Результат роботи модуля з синхронізацією та з уведеними thread_num=10, thread_inccnt=500, thread_delay=0:

.. code-block:: bash

 / # insmod /mnt/with_synch.ko thread_num=10 thread_inccnt=500 thread_delay=0
 [  173.153555] Modue with_synch is installed!
 [  173.153756] init glob_var = 0
 [  173.155290] thread-0 was created suceccfuly!
 [  173.156223] thread-1 was created suceccfuly!
 [  173.157829] thread-2 was created suceccfuly!
 [  173.158413] thread-3 was created suceccfuly!
 [  173.158976] thread-4 was created suceccfuly!
 [  173.159512] thread-5 was created suceccfuly!
 [  173.159887] thread-6 was created suceccfuly!
 [  173.160714] thread-7 was created suceccfuly!
 [  173.161265] thread-8 was created suceccfuly!
 [  173.162502] thread-9 was created suceccfuly!
 / # rmmod with_synch
 [  194.497776] thread-0 was stoped
 [  194.498129] thread-1 was stoped
 [  194.498277] thread-2 was stoped
 [  194.498401] thread-3 was stoped
 [  194.498560] thread-4 was stoped
 [  194.498705] thread-5 was stoped
 [  194.498851] thread-6 was stoped
 [  194.499416] thread-7 was stoped
 [  194.499587] thread-8 was stoped
 [  194.499736] thread-9 was stoped
 [  194.499938] final glob_var = 5000
 [  194.500474] glob_var = 4742
 [  194.500649] glob_var = 4820
 [  194.500781] glob_var = 4863
 [  194.500950] glob_var = 4878
 [  194.501213] glob_var = 4880
 [  194.501392] glob_var = 4885
 [  194.501520] glob_var = 4912
 [  194.501645] glob_var = 4916
 [  194.501771] glob_var = 4998
 [  194.501918] glob_var = 5000
 [  194.502154] Modue with_synch is removed
 / # 

Результат збігається з очікуваним.

Результат роботи модуля з власними реалізаціями lock(), unlock та з уведеними thread_num=10, thread_inccnt=500, thread_delay=0:

.. code-block:: bash

 / # insmod /mnt/my_synch.ko thread_num=10 thread_inccnt=500 thread_delay=0
 [  271.614800] Modue my_synch is installed!
 [  271.615226] init glob_var = 0
 [  271.616325] thread-0 was created suceccfuly!
 [  271.617128] thread-1 was created suceccfuly!
 [  271.617964] thread-2 was created suceccfuly!
 [  271.631536] thread-3 was created suceccfuly!
 [  271.632419] thread-4 was created suceccfuly!
 [  271.640177] thread-5 was created suceccfuly!
 [  271.662649] thread-6 was created suceccfuly!
 [  271.671685] thread-7 was created suceccfuly!
 [  271.677277] thread-8 was created suceccfuly!
 [  271.693119] thread-9 was created suceccfuly!
 / # rmmod my_synch
 [  296.092278] thread-0 was stoped
 [  296.092466] thread-1 was stoped
 [  296.092565] thread-2 was stoped
 [  296.092700] thread-3 was stoped
 [  296.092804] thread-4 was stoped
 [  296.093045] thread-5 was stoped
 [  296.093164] thread-6 was stoped
 [  296.093289] thread-7 was stoped
 [  296.093510] thread-8 was stoped
 [  296.093635] thread-9 was stoped
 [  296.093770] final glob_var = 5000
 [  296.093901] glob_var = 3705
 [  296.094123] glob_var = 4296
 [  296.094218] glob_var = 4324
 [  296.094303] glob_var = 4553
 [  296.094451] glob_var = 4586
 [  296.094535] glob_var = 4699
 [  296.094650] glob_var = 4806
 [  296.094737] glob_var = 4926
 [  296.094829] glob_var = 4997
 [  296.095002] glob_var = 5000
 [  296.095109] Modue my_synch is removed
 / # [  302.725256] random: crng init done

Результат збігається з очікуваним.

Результат роботи модуля при досрочній вигрузці модуля:

.. code-block:: bash

 / # insmod /mnt/my_synch.ko thread_num=10 thread_inccnt=500 thread_delay=0
 [  383.615961] Modue my_synch is installed!
 [  383.616293] init glob_var = 0
 [  383.617320] thread-0 was created suceccfuly!
 [  383.621465] thread-1 was created suceccfuly!
 [  383.622439] thread-2 was created suceccfuly!
 [  383.623217] thread-3 was created suceccfuly!
 [  383.623649] thread-4 was created suceccfuly!
 [  383.624127] thread-5 was created suceccfuly!
 [  383.626609] thread-6 was created suceccfuly!
 [  383.633405] thread-7 was created suceccfuly!
 [  383.646453] thread-8 was created suceccfuly!
 [  383.654496] thread-9 was created suceccfuly!
 / # rmmod my_synch
 [  386.078436] thread-0 was stoped
 [  386.152241] thread-1 was stoped
 [  386.216179] thread-2 was stoped
 [  386.216976] thread-3 was stoped
 [  386.281462] thread-4 was stoped
 [  386.281488] thread-5 was stoped
 [  386.287140] thread-6 was stoped
 [  386.292113] thread-7 was stoped
 [  386.294493] thread-8 was stoped
 [  386.296279] thread-9 was stoped
 [  386.298579] final glob_var = 2020
 [  386.300608] glob_var = 1899
 [  386.302051] glob_var = 1947
 [  386.302676] glob_var = 1991
 [  386.303660] glob_var = 1992
 [  386.304793] glob_var = 2011
 [  386.306010] glob_var = 2012
 [  386.306625] glob_var = 2013
 [  386.308248] glob_var = 2017
 [  386.308874] glob_var = 2018
 [  386.310023] glob_var = 2020
 [  386.311291] Modue my_synch is removed
 / # 

**Використані бібліотеки**

* ``<linux/module.h>`` - потрібна для всіх модулів   
* ``<linux/moduleparam.h>`` - для використання макросів параметрів     
* ``<linux/kernel.h>`` - заголовки ядра    
* ``<linux/init.h>`` - використовується для ініціалізації та деініціалізації 
* ``<linux/list.h>`` - необхідна для ініціалізації та використання списків
* ``<linux/kthread.h>`` - необхідня для роботи з потоками ядра
* ``<linux/sched/task.h>`` - необхідна для перемикання між задачами
* ``<linux/mutex.h>`` - необхідна для синхронізації потоків між собою
* ``<linux/bitops.h>`` - необхідна для атомарних побітових операцій


**Використані макроси та функції**


``MODULE_DESCRIPTION`` - макрос для опису модуля    

``MODULE_AUTHOR`` - автор модуля    

``MODULE_VERSION`` - версія модуля    

``MODULE_LICENSE`` - тип ліцензії  

``module_param`` - передача параметрів в модуль    

``MODULE_PARM_DESC`` - опис параметра

``LIST_HEAD`` - створення списку, який ініціалізується під час компіляції

``INIT_LIST_HEAD`` - ініціалізує список з одного елемента

``DEFINE_MUTEX`` - статично визначає mutex

``kthread_run`` - створює та запускає потік

``kthread_stop`` - зупиняє потік

``get_task_struct`` - задає потоку нову задачу на виконання

``put_task_struct`` - зніма з потоку задачу на виконання

``list_add_tail`` - додає у кінець списку новий елемент

``list_del`` - видаляє елемент зі списку

``list_for_each_entry_safe`` - проходить по усім елементам списку з можливістю одночасного їх видалення

``mutex_lock`` - забороняє іншим потокам виконувати наступний код

``mutex_unlock`` - дозволяє наступному потоку виконувати наступний код

``test_and_set_bit`` - встановлює біт в 1 та повертає його попереднє значення

``clear_bit`` - встановлює біт в 0

``schedule`` - перемикає на іншу задачу

``schedule_timeout_uninterruptible`` - реалізує затримку у значеннях jiffies без дозволу на переривання

Висновки:
-------------

В результаті виконання даної лабораторної роботи було написано модуль, який створює thread_num потоків, кожен з яких інкрементує значення глобальної змінної glob_var thread_inccnt разів з затрімкою в thread_delay. В результаті перевіркі роботи модуля було з'ясовано, що модуль працює коректно при наявній синхронізації та не вірно прі її відсутності.
Також було з'ясовано, що модуль з власними реалізаціями lock(), unlock() працює коректно.