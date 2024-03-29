*Лабораторна робота №2. Tasklets. High resolutions timers*.

	*Завдання*:

    * Написати модуль який приймає два аргументи: count, delay.

 При ініціалізації  модуль виводить значення Jiffies, створює масив розміром count, запускає tasklet, який виводить значення jiffies та запускає hrtimer із затримкою delay, який записує кількість значень count Jiffies у масив. При розвантаженні модуль знову виводить значення jiffies та друкує  масив.


	*Хід роботи*:

Для початку потрібно створити два параметри ``count``  та ``delay``.

Наступним кроком було створено файл конфігурації лінукс ядра, його налагодження та сбірка. На етапі збіркі було зібрано тільки ядро.

Створено массив ``jif_arr`` розміром count куди буде записуватися значення jiffies при виклику  обробника таймеру ``arr_timer``. Алокація пам'яті до  масиву відбувається за допомогою kzalloc.


Далі створено сам таймер arr_timer та його обробник. У функції обробнику значення Jiffies буде записуватись ``count`` разів до масиву ``jiff_arr``. Для об'явлення таймеру використав ``DEFINE_TIMER``.

Також створено тасклет ``my_tasklet`` та його обробник ``tasklet_handler``

  *Об'явлення тасклету відбувається за допомогою DECLARE_TASKLET

  * Для запуска у режимі звичайного приорітету використовується ``tasklet_schedule``

У init відбувається перевірка вхідних значень на правильність. Якщо все вірно уведено - відбувається ініціалізація масиву, тасклетку та запуск таймеру.
	
У exit відбувається виведення значення масиву, поточне значення ``jiffies``
Та очищення ресурсів:

	* Массиву ``kfree(jiff_arr);``
	* Тасклету ``tasklet_kill(&my_tasklet); ``
	* Таймеру ``hrtimer_cancel(&arr_timer); ``

	*Результат роботи модуля*:

*У якості прикладу count=4; delay=1000*

.. code-block:: bash

 / # insmod /mnt/secondmod.ko count=4 delay=1000
 [   66.261209] secondmod: jiff = 4294733575
 [   66.261843] secondmod: tasklet 
 [   66.261872] secondmod: jiff = 4294733575
 / # rmmod secondmod
 [   70.272978] secondmod : count finished
 [   75.259719] secondmod : jiff = 4294742573
 [   75.261049] secondmod : arr[0] = 4294734576
 [   75.261234] secondmod : arr[1] = 4294735601
 [   75.261402] secondmod : arr[2] = 4294736584
 [   75.261610] secondmod : arr[3] = 4294737587


*При завчасному вивантаженню модуля виведеться відповідне повідомлення та ті елементи массиву які вдалося записати*


.. code-block:: bash
 [  325.341571] secondmod: jiff = 4294992655
 [  325.342270] secondmod: tasklet 
 [  325.342298] secondmod: jiff = 4294992656
 / # rmmod secondmod
 [  327.156300] secondmod : jiff = 4294994470
 [  327.156625] secondmod: timer is dead

*При введенні значення count,delay=0*

.. code-block:: bash
 / # insmod /mnt/secondmod.ko count=0 delay=0
 [  645.081221] secondmod: jiff = 4295312395
 [  645.081491] secondmod: count value not correct 0

*Висновок*
В результаті виконання даної лабораторної роботи було написано модуль, який ініціалізує та виконує тасклет, запускає hr таймер та виділяє пам'ять для масиву jiff_arr, що має розмірність count. В результаті перевірки роботи модуля було з'ясовано, що модуль працює коректно та правильно виконує перевірку на правильність уведених в нього параметрів.