=====================================
**Лабораторна робота №3 Потоки ядра**
=====================================

Завдання:
---------------
Написати модуль ядра, який:
	* приймає 3 параметри *thread_num*, *thread_inccnt* та *thread_delay*;
	* запускає *thread_num* потоків на одночасне виконання;
	* кожен поток інкрементує глобальну змінну *glob_var* (*thread_inccnt* разів) із затримкою *thread_delay*;
	* зберігає отримане значення змінної у список;
	* при вивантаженні модуль повинен виводити остаточне значення *glob_var* та вміст списку.

**Хід роботи**

Спочатку було створено список *glob_var_list*, масив потоків *thread* та структуру *list_node*, яка наслідує структуру *list_head* та зберігатиме значення змінної glob_var:

.. code-block:: bash
 static LIST_HEAD(glob_var_list);
 static struct task_struct **thread = NULL;
 struct list_node {
 	struct list_head list;
 	unsigned long glob_var_val;
 };
Після чого було створено функцію *thread_func*, яка виконуватиметься у потоці:

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
У функції *__init thirdmod_init* було написано блок перевірки правильності значень уведених параметрів модуля, та перевірка на успішність виділення пам'яті під масив *thread*:

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
--------------------------------------

Результат роботи модуля, без уведених аргументів:

.. code-block:: bash
 / # insmod /mnt/mod_3_no_synch.ko
    [  190.367281] mod_3_no_synch: loading out-of-tree module taints kernel.
    [  190.395658] Modue mod_3_no_synch is installed!
    [  190.401379] init glob_var = 0
    [  190.411425] thread-0 was created suceccfuly!
    [  190.417108] thread-1 was created suceccfuly!
    [  190.419627] thread-2 was created suceccfuly!
    [  190.424313] insmod (99) used greatest stack depth: 13696 bytes left
    / # rmmod mod_3_no_synch
    [  224.282669] thread-0 was stoped
    [  224.284173] thread-1 was stoped
    [  224.285646] thread-2 was stoped
    [  224.287078] final glob_var = 30
    [  224.288373] glob_var = 28
    [  224.289617] glob_var = 29
    [  224.290404] glob_var = 30
    [  224.291327] Modue mod_3_no_synch is removed
За замовчуванням, кількість потоків рівна 3, кількість інкрементацій 10 і затримка рівна 50 мілісекунді.

Результат роботи модуля без синхронізації з уведеними thread_num=8, thread_inccnt=400, thread_delay=0:

.. code-block:: bash
 / # insmod /mnt/mod_3_no_synch.ko thread_num=8 thread_inccnt=400 thread_delay=0
    [  498.319787] Modue mod_3_no_synch is installed!
    [  498.320219] init glob_var = 0
    [  498.328817] thread-0 was created suceccfuly!
    [  498.333221] thread-1 was created suceccfuly!
    [  498.334889] thread-2 was created suceccfuly!
    [  498.336069] thread-3 was created suceccfuly!
    [  498.337109] thread-4 was created suceccfuly!
    [  498.338263] thread-5 was created suceccfuly!
    [  498.339727] thread-6 was created suceccfuly!
    [  498.340660] thread-7 was created suceccfuly!
    / # rmmod mod_3_no_synch
    [  505.352412] thread-0 was stoped
    [  505.353634] thread-1 was stoped
    [  505.353881] thread-2 was stoped
    [  505.354154] thread-3 was stoped
    [  505.354567] thread-4 was stoped
    [  505.354829] thread-5 was stoped
    [  505.355054] thread-6 was stoped
    [  505.355496] thread-7 was stoped
    [  505.355776] final glob_var = 3199
    [  505.355951] glob_var = 2688
    [  505.356184] glob_var = 3044
    [  505.356739] glob_var = 3125
    [  505.356984] glob_var = 3131
    [  505.357089] glob_var = 3155
    [  505.357267] glob_var = 3183
    [  505.357811] glob_var = 3185
    [  505.358025] glob_var = 3199
    [  505.358453] Modue mod_3_no_synch is removed
Як видно, результат не збігається з очікуваним, який має бути 3200, натомість отримали результат 3199.

Результат роботи модуля з синхронізацією та з уведеними thread_num=8, thread_inccnt=400, thread_delay=0:

.. code-block:: bash
 / # insmod /mnt/mod_3_with_synch.ko thread_num=8 thread_inccnt=400 thread_delay=0
    [  620.491108] random: crng init done
    [  620.522872] Modue mod_3_with_synch is installed!
    [  620.523484] init glob_var = 0
    [  620.525536] thread-0 was created suceccfuly!
    [  620.529806] thread-1 was created suceccfuly!
    [  620.536313] thread-2 was created suceccfuly!
    [  620.558667] thread-3 was created suceccfuly!
    [  620.571449] thread-4 was created suceccfuly!
    [  620.585432] thread-5 was created suceccfuly!
    [  620.593804] thread-6 was created suceccfuly!
    [  620.598737] thread-7 was created suceccfuly!
    / # rmmod mod_3_with_synch
    [  633.901787] thread-0 was stoped
    [  633.905010] thread-1 was stoped
    [  633.905504] thread-2 was stoped
    [  633.905716] thread-3 was stoped
    [  633.905937] thread-4 was stoped
    [  633.906164] thread-5 was stoped
    [  633.906437] thread-6 was stoped
    [  633.906734] thread-7 was stoped
    [  633.907113] final glob_var = 3200
    [  633.907524] glob_var = 2939
    [  633.907790] glob_var = 2947
    [  633.907984] glob_var = 3041
    [  633.908138] glob_var = 3092
    [  633.908469] glob_var = 3095
    [  633.908701] glob_var = 3102
    [  633.908878] glob_var = 3160
    [  633.909007] glob_var = 3200
    [  633.909159] Modue mod_3_with_synch is removed
Результат збігається з очікуваним.

Результат роботи модуля з власними реалізаціями lock(), unlock та з уведеними thread_num=8, thread_inccnt=400, thread_delay=0:

.. code-block:: bash
 / # insmod /mnt/mod_3_my_synch.ko thread_num=8 thread_inccnt=400 thread_delay=0
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
 / # rmmod mod_3_my_synch
 [  296.092278] thread-0 was stoped
 [  296.092466] thread-1 was stoped
 [  296.092565] thread-2 was stoped
 [  296.092700] thread-3 was stoped
 [  296.092804] thread-4 was stoped
 [  296.093045] thread-5 was stoped
 [  296.093164] thread-6 was stoped
 [  296.093289] thread-7 was stoped
  [  296.093770] final glob_var = 3200
 [  296.093901] glob_var = 2935
 [  296.094123] glob_var = 2946
 [  296.094218] glob_var = 2988
 [  296.094303] glob_var = 3025
 [  296.094451] glob_var = 3056
 [  296.094535] glob_var = 3102
 [  296.094650] glob_var = 3173
 [  296.094737] glob_var = 3200
  [  296.095109] Modue my_synch is removed
 / # [  302.725256] random: crng init done
Результат збігається з очікуваним.

Результат роботи модуля при досрочній вигрузці модуля:

.. code-block:: bash
/ # insmod /mnt/mod_3_my_synch.ko thread_num=8 thread_inccnt=400 thread_delay=0
    [ 1017.146903] Modue mod_3_my_synch is installed!
    [ 1017.147516] init glob_var = 0
    [ 1017.149021] thread-0 was created suceccfuly!
    [ 1017.150898] thread-1 was created suceccfuly!
    [ 1017.156178] thread-2 was created suceccfuly!
    [ 1017.159302] thread-3 was created suceccfuly!
    [ 1017.171185] thread-4 was created suceccfuly!
    [ 1017.221104] thread-5 was created suceccfuly!
    [ 1017.227423] thread-6 was created suceccfuly!
    [ 1017.246720] thread-7 was created suceccfuly!
    / # rmmod mod_3_my_synch
    [ 1019.097469] thread-0 was stoped
    [ 1019.690811] thread-1 was stoped
    [ 1019.787505] thread-2 was stoped
    [ 1019.998469] thread-3 was stoped
    [ 1020.124484] thread-4 was stoped
    [ 1020.125141] thread-5 was stoped
    [ 1020.161209] thread-6 was stoped
    [ 1020.172197] thread-7 was stoped
    [ 1020.172968] final glob_var = 335
    [ 1020.173687] glob_var = 204
    [ 1020.174128] glob_var = 263
    [ 1020.174641] glob_var = 270
    [ 1020.174936] glob_var = 290
    [ 1020.175491] glob_var = 321
    [ 1020.175909] glob_var = 322
    [ 1020.176288] glob_var = 334
    [ 1020.176817] glob_var = 335
    [ 1020.177420] Modue mod_3_my_synch is removed
