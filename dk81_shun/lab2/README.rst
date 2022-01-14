============================================================
**Лабораторна робота №2 Використання таймерів та тасклетів**
============================================================

Завдання:
---------------
Написати модуль ядра, який виконує наступні дії:

* Приймає два параметри cnt та delay
* Виводить поточне значення jiffies у init та exit функціях 
* Запускає тасклет, який вивдить своє поточне значення jiffies
* Виділяє масив розміром cnt, у якому будуть зберігатися значення jiffies
* Запустити hrtimer, при спрацюванні якого у масив буде записуватися поточне значення jiffies

**Хід роботи**

**Використані бібліотеки**

* ``<linux/module.h>`` - потрібна для всіх модулів   
* ``<linux/moduleparam.h>`` - для використання макросів параметрів     
* ``<linux/kernel.h>`` - заголовки ядра    
* ``<linux/init.h>`` - використовується для ініціалізації та деініціалізації    
* ``<linux/jiffies.h>`` - лічильник
* ``<linux/interrupt.h>`` - необхідна для використанні тасклетів
* ``<linux/slab.h>`` - необхідна для виділення пам'яті
* ``<linux/hrtimer.h>`` - необхідна для використання hr тамерів

**Використані макроси та функції**


``MODULE_DESCRIPTION`` - макрос для опису модуля    

``MODULE_AUTHOR`` - автор модуля    

``MODULE_VERSION`` - версія модуля    

``MODULE_LICENSE`` - тип ліцензії  

``module_param`` - передача параметрів в модуль    

``MODULE_PARM_DESC`` - опис параметра

``printk`` - виведення інформації в лог ядра

``HRTIMER_RESTART`` - перезавантажувати таймер

``HRTIMER_NORESTART`` - не перезавантажувати таймер

``CLOCK_MONOTONIC`` - в якості джерела тактів використовувати CLOCK_MONOTONIC

``HRTIMER_MODE_REL`` - відлік часу починається з моменту ініціалізації таймера 
    
``GFP_KERNEL`` - виділення пам'яті у адресному просторі ядра

``kzalloc`` - виділення пам'ті в адресному просторі ядра, з обнуленням її вмісту

``enum hrtimer_restart timer_handler`` - функція-обробник переривань таймера

``hrtimer_init`` - ініціалізація таймера

``hrtimer_start`` - запуск таймера

``tasklet_schedule`` - призначення тасклету функції обробника

``kfree`` - вивільнення пам'яті

``hrtimer_cancel`` - припинення роботи таймера

``tasklet_kill`` - припинення роботи тасклета

Спершу було створено тасклет за допомогою макроса DECLARE_TASKLET та створено функцію, яка викликатиметься при спрацюванні тасклета - tasklet_handler:

.. code-block:: bash

 static void tasklet_handler(struct tasklet_struct *data);

 DECLARE_TASKLET(task, tasklet_handler);
 
 static void tasklet_handler(struct tasklet_struct *data)
 {
     printk(KERN_INFO "Tasklet jiffies = %lu\n", jiffies);
 }


Далі було створено таймер як змінну типу struct hrtimer, та створено функцію, яка викликається при спрацюванні таймера:

.. code-block:: bash

 static struct hrtimer cnt_timer;

 static enum hrtimer_restart timer_handler(struct hrtimer *timer)
 {
     jiff_arr[i] = jiffies;
     i++;
 
     if (i == cnt)
         return HRTIMER_NORESTART;
 
     hrtimer_forward_now(&cnt_timer, ms_to_ktime(delay));
     return HRTIMER_RESTART;
 }

У функції __init secondmod_init було написано блок перевірки правильності значень уведених параметрів модуля, та перевірка на успішність виділення пам'яті під масив jiff_arr, також відбувається прізначення тасклету функції обробника:

.. code-block:: bash

     printk(KERN_INFO "Init jiffies = %lu\n", jiffies);
     tasklet_schedule(&task);
     if (cnt <= 0) {
         printk(KERN_ERR "Error: invalid value of cnt!\n");
         goto final;
     }
     if (delay < 0) {
         printk(KERN_ERR "Error: invalid value of delay!\n");
         goto final;
     }
 
     jiff_arr = kzalloc(cnt * sizeof(*jiff_arr), GFP_KERNEL);
     if (jiff_arr == NULL) {
         printk(KERN_ERR "Error: cannot allocate memory!\n");
         status = -ENOMEM;
         goto final;
     }
   ...
 
 final:
 return status;

Після перевірки йде ініціализація та запуск тамера:

.. code-block:: bash
 
 hrtimer_init(&cnt_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
 cnt_timer.function = &timer_handler;
 hrtimer_start(&cnt_timer, delay, HRTIMER_MODE_REL);

У функції __exit secondmod_exit відповидно вивільняється пам'ять масива та припиняється робота такмера і тасклета.

.. code-block:: bash

 if (jiff_arr == NULL) {
         goto exit_final;
     }
     if (hrtimer_is_queued(&cnt_timer)) {
         cnt = i;
         printk(KERN_INFO "array filling stoped!");
     }
     while (i > 0) {
         printk(KERN_INFO "jiff_arr[%lu] = %lu\n",
         cnt - i, jiff_arr[cnt - i]);
         i--;
     }
     kfree(jiff_arr);
     hrtimer_cancel(&cnt_timer);
 exit_final:
     tasklet_kill(&task);

**Результат роботи створеного модуля**

Результат роботи модуля, без уведеного аргумента:

.. code-block:: bash

 / # insmod /mnt/secondmod.ko
 [   98.474398] Init jiffies = 4294765552
 [   98.475526] Tasklet jiffies = 4294765553
 / # rmmod secondmod
 [  105.119380] Exit jiffies = 4294772197
 [  105.120774] jiff_arr[0] = 4294765555
 / # 

За замовчуванням, кількість комірок масива рівна 1, а затримка 1 мілісекунда.

Результат роботи модуля з уведеним cnt = 0:

.. code-block:: bash

 / # insmod /mnt/secondmod.ko cnt=0
 [  205.172371] Init jiffies = 4294872250
 [  205.173498] Error: invalid value of cnt!
 [  205.174361] Tasklet jiffies = 4294872252
 / # rmmod secondmod
 [  209.895415] Exit jiffies = 4294876973
 / # 

Результат роботи модуля з уведеним delay = 0:

.. code-block:: bash

 / # insmod /mnt/secondmod.ko cnt=10 delay=0
 [ 1004.230265] Init jiffies = 4295671308
 [ 1004.231238] Tasklet jiffies = 4295671309
 / # rmmod secondmod
 [ 1005.639344] Exit jiffies = 4295672717
 [ 1005.640784] jiff_arr[0] = 4295671310
 [ 1005.641733] jiff_arr[1] = 4295671310
 [ 1005.642652] jiff_arr[2] = 4295671310
 [ 1005.644123] jiff_arr[3] = 4295671311
 [ 1005.644938] jiff_arr[4] = 4295671311
 [ 1005.645746] jiff_arr[5] = 4295671311
 [ 1005.646544] jiff_arr[6] = 4295671317
 [ 1005.647295] jiff_arr[7] = 4295671317
 [ 1005.647533] jiff_arr[8] = 4295671317
 [ 1005.648842] jiff_arr[9] = 4295671318
 / # 

Результат роботи модуля при його вивантаженні під час заповнення масиву:

.. code-block:: bash

 / # insmod /psh/lab2_mod.ko cnt=10 delay=1000
 [  294.908266] Init jiffies = 4294961986
 [  294.909187] Tasklet jiffies = 4294961987
 / # rmmod secondmod
 [  299.382107] Exit jiffies = 4294966460
 [  299.383013] array filling stoped!
 [  299.383049] jiff_arr[0] = 4294961992
 [  299.384549] jiff_arr[1] = 4294962989
 [  299.385264] jiff_arr[2] = 4294963989
 [  299.386042] jiff_arr[3] = 4294964989
 [  299.391209] jiff_arr[4] = 4294965989
 / # 

Результат роботи модуля при вірно заданих параметрах та повному заповненні масива:

.. code-block:: bash

 / # insmod /psh/lab2_mod.ko cnt=10 delay=100
 [  360.044600] Init jiffies = 4295027123
 [  360.045383] Tasklet jiffies = 4295027123
 / # rmmod secondmod
 [  364.408531] Exit jiffies = 4295031486
 [  364.411480] jiff_arr[0] = 4295027125
 [  364.413612] jiff_arr[1] = 4295027225
 [  364.414383] jiff_arr[2] = 4295027325
 [  364.415100] jiff_arr[3] = 4295027425
 [  364.415805] jiff_arr[4] = 4295027525
 [  364.416426] jiff_arr[5] = 4295027625
 [  364.417420] jiff_arr[6] = 4295027725
 [  364.420438] jiff_arr[7] = 4295027825
 [  364.421140] jiff_arr[8] = 4295027925
 [  364.421847] jiff_arr[9] = 4295028025
 / # 

**Відповіді на питання**

1. Різниця значень jiffies відрізняється на 0, якщо ядро не навантажено і може виконати тасклет не очікуючи наступного такту. Різниця значень jiffies відрізняється на 1, якщо ядро авантажено, в такому разі ядро вже буде очікувати наступний такт. Різниця значень jiffies відрізняється на 2, якщо ядро навантажене і перед цим бло перемикання контексту, в такому разі scrdule виконається при наступному такті, а виконання тасклету ще на наступному такті, загалом виходить два такти.

2. У даній роботі було використано прапор GFP_KERNEL при виділенни пам'яті, оскільки, виділення відбувається не у перериванні і ядро може призупинити поточний процес, очикуючи поки буде виділено вільну пам'ять.

3. Різниця jiffies між двома запусками таймера рівна затримці, заданій у мілісекундах, оскільки, частота тактів системного таймера рівна 1000 Гц, що відповідає періоду в 1 мс.

4. При delay = 0 значення jiffies у всіх комірках масива будуть однаковими, оскільки, таймер буде миттєво перезавантажуватися і відповідно функція обробки буде викликатися миттєво, не очікуючи наступного такту системного таймера.


Висновки:
-------------

В результаті виконання даної лабораторної роботи було написано модуль, який ініціалізує та виконує тасклет, ініціалізує та запускає hr таймер та виділяє пам'ять для масиву 
jiff_arr, який має розмирмирність cnt. В результаті перевірки роботи модуля було з'ясовано, що модуль працює коректно та правильно виконує перевірку на правильність уведених в нього параметрів.
