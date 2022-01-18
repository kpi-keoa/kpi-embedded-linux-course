**Лабораторна робота №3**
     Threads & Lists
 ==========================

**Завдання:**
 ==========================

Написати модуль ядра, що підпадає вимогам:

	- Має у собі глобальну змінную ``glob_var`` та параметри `` thread_num, thread_inccnt, 
	thread_delay ``
	- Запускає ``thread_num`` потоків на одночасне виконання
	- Кожен потік інкрементує змінну ``glob_var`` ``thread_inccnt`` раз з затримкою ``thread_delay
	``, кладе значення змінної в список 
	- При вивантаженні модуль повинен виводити остаточне значення ``glob_var`` та вміст списку

	
**Хід роботи:**

Спершу було створено список glob_var_list, масив потоків thread та структуру list_node, що наслідує структуру list_head та зберігатиме значення змінної glob_var. також створена функція, що виконується у потоці.

Наступним кроком у функції __init thirdmod_init було написано блок перевірки правильності значень уведених параметрів модуля, та перевірка на успішність виділення пам'яті під масив thread

``int i;

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
init_final: return status;``

Далі відбувається запуск кожного з потоків та у функції __exit thirdmod_exit відповідно зупиняються потоки, вивільняється пам'ять масива та виводиться вміст списку та значення glob_var

``int i; struct list_node *pos = NULL; struct list_node *n;

   if(NULL == thread)
           goto exit_final;

   for (i = 0; i < thread_num; i++)
   {
           if(NULL == thread[i])
                   goto skip;

           kthread_stop(thread[i]);
           put_task_struct(thread[i]);
skip: pr_info("thread-%d was stoped\n", i); } kfree(thread);

   list_for_each_entry_safe(pos, n, &glob_var_list, list)
   {
           pr_info("glob_var = %lu\n", pos->glob_var_val);
           list_del(&(pos->list));
           kfree(pos);
   }

   pr_info("final glob_var = %lu\n", glob_var);
exit_final: pr_info("Modue %s is removed\n", module_name(THIS_MODULE));``

**Результат роботи модуля:**

Результат (без синхронізації) з параметрами thread_num=10, thread_inccnt=600, thread_delay=0:

# insmod /mnt/lab3_w_sync.ko thread_num=10 thread_inccnt=600 thread_delay=0 
[ 133.629935] Modue lab3_w_sync is installed! 
[ 133.630120] init glob_var = 0 
[ 133.631488] thread-0 was created suceccfuly! 
[ 133.633412] thread-2 was created suceccfuly! 
[ 133.633961] thread-3 was created suceccfuly! 
[ 133.634867] thread-4 was created suceccfuly! 
[ 133.635454] thread-5 was created suceccfuly! 
[ 133.636006] thread-6 was created suceccfuly! 
[ 133.636777] thread-7 was created suceccfuly! 
[ 133.637685] thread-8 was created suceccfuly! 
[ 133.638341] thread-9 was created suceccfuly! 
/ # rmmod lab3_w_sync 
[ 137.350509] thread-0 was stoped 
[ 137.350509] thread-1 was stoped 
[ 137.350509] thread-2 was stoped 
[ 137.351586] thread-3 was stoped 
[ 137.351744] thread-4 was stoped 
[ 137.351824] thread-5 was stoped 
[ 137.351896] thread-6 was stoped 
[ 137.351969] thread-7 was stoped 
[ 137.352043] thread-8 was stoped 
[ 137.352383] thread-9 was stoped 
[ 137.352611] final glob_var = 5997 
[ 137.352748] glob_var = 5723 
[ 137.352867] glob_var = 5768 
[ 137.352935] glob_var = 5819 
[ 137.353001] glob_var = 5922 
[ 137.353142] glob_var = 5948 
[ 137.353488] glob_var = 5950 
[ 137.353569] glob_var = 5961 
[ 137.353658] glob_var = 5982 
[ 137.353748] glob_var = 5989 
[ 137.353825] glob_var = 5997 
[ 137.353952] Modue lab3_w_sync removed / #

Результат роботи модуля з власними реалізаціями lock(), unlock та з параметрами thread_num=10, thread_inccnt=600, thread_delay=0

/ # insmod /mnt/lab3_own_sync.ko thread_num=10 thread_inccnt=600 thread_del ay=0 
[ 227.989670] Modue lab3_own_sync is installed! 
[ 227.989930] init glob_var = 0 
[ 227.991103] thread-0 was created suceccfuly! 
[ 227.992601] thread-1 was created suceccfuly! 
[ 227.993961] thread-2 was created suceccfuly! 
[ 227.995751] thread-3 was created suceccfuly! 
[ 227.996682] thread-4 was created suceccfuly! 
[ 228.003563] thread-5 was created suceccfuly! 
[ 228.011185] thread-6 was created suceccfuly! 
[ 228.014761] thread-7 was created suceccfuly! 
[ 228.024050] thread-8 was created suceccfuly! 
[ 228.037819] thread-9 was created suceccfuly! / # 
rmmod lab3_own_sync 
[ 239.201615] thread-0 was stoped 
[ 239.201615] thread-1 was stoped 
[ 239.201615] thread-2 was stoped 
[ 239.202760] thread-3 was stoped 
[ 239.202839] thread-4 was stoped 
[ 239.202911] thread-5 was stoped 
[ 239.202981] thread-6 was stoped 
[ 239.203051] thread-7 was stoped 
[ 239.203118] thread-8 was stoped 
[ 239.203498] thread-9 was stoped 
[ 239.203759] final glob_var = 6000 
[ 239.204395] glob_var = 4825 
[ 239.204542] glob_var = 5324 
[ 239.204613] glob_var = 5451 
[ 239.204691] glob_var = 5514 
[ 239.204766] glob_var = 5766 
[ 239.204831] glob_var = 5810 
[ 239.204901] glob_var = 5839 
[ 239.204972] glob_var = 5979 
[ 239.205040] glob_var = 5987 
[ 239.205345] glob_var = 6000 
[ 239.205462] Modue lab3_own_lock removed / #

Результат роботи модуля з синхронізацією та з параметрами thread_num=10, thread_inccnt=300, thread_delay=0:

/ # insmod /mnt/lab3_sync.ko thread_num=10 thread_inccnt=300 thread_delay=0 
[ 432.408712] Modue lab3_lock is installed! 
[ 432.408889] init glob_var = 0 
[ 432.409826] thread-0 was created suceccfuly! 
[ 432.410618] thread-1 was created suceccfuly! 
[ 432.411510] thread-2 was created suceccfuly! 
[ 432.412100] thread-3 was created suceccfuly! 
[ 432.412817] thread-4 was created suceccfuly! 
[ 432.413588] thread-5 was created suceccfuly! 
[ 432.414068] thread-6 was created suceccfuly! 
[ 432.414684] thread-7 was created suceccfuly! 
[ 432.415375] thread-8 was created suceccfuly! 
[ 432.415906] thread-9 was created suceccfuly! / # 
rmmod lab3_sync 
[ 436.752441] thread-0 was stoped 
[ 436.752643] thread-1 was stoped 
[ 436.752722] thread-2 was stoped 
[ 436.752843] thread-3 was stoped 
[ 436.752917] thread-4 was stoped 
[ 436.752989] thread-5 was stoped 
[ 436.753077] thread-6 was stoped 
[ 436.753165] thread-7 was stoped 
[ 436.753574] thread-8 was stoped 
[ 436.753759] thread-9 was stoped 
[ 436.753899] final glob_var = 3000 
[ 436.753987] glob_var = 2735 
[ 436.754111] glob_var = 2760 
[ 436.754629] glob_var = 2765 
[ 436.754838] glob_var = 2769 
[ 436.755053] glob_var = 2774 
[ 436.755368] glob_var = 2782 
[ 436.755443] glob_var = 2788 
[ 436.755531] glob_var = 2908 
[ 436.755618] glob_var = 2911 
[ 436.755689] glob_var = 3000 
[ 436.755847] Modue lab3_sync is removed

**Висновок:**
 У даній лабораторній роботі було написано модуль, який створює thread_num потоків, кожен з яких інкрементує значення глобальної змінної glob_var у кількості разів thread_inccnt з затримкою в thread_delay. Результати показують, що без синхронізації значення не зовсім корректні, а із власною реалізацією lock() та unlock() працює корректно.