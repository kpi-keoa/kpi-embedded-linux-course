Лабораторна робота 2
====================

Завдання
--------
Написати модуль який приймає два аргументи: cnt, delay.

При ініціалізації (module_init) модуль виводить значення jiffies, створює масив розміром cnt, запускає tasklet який виводить значення jiffies та запускає hrtimer із затримкою delay мс, що записує cnt значень jiffies у масив. При розвантаженні модуль знову виводить значення jiffies та друкує отриманий масив.

Xiд роботи
----------

tasket
~~~~~~
Для того щоб створити tasklet необхідно:

створити callback-функцію обробник;
використати макрос DECLARE_TASKLET
АБО

ініціалізувати tasklet вручну за допомогою об'явлення структури struct tasklet_struct <name> та функції tasklet_init;
Далі можемо "запланувати" виконання tasklet'у за допомогою tasklet_schedule.

hrtimer
~~~~~~~
Для створення hrtimer'у необхідно:

оголосити структуру struct hrtimer <name>;
оголосити функцію-обробник типу enum hrtimer_restart <function_name>(struct hrtimer *timer);
ініціалізувати оголошену раніше структуру за допомогою hrtimer_init;
Наступним кроком можемо запустити таймер функцією hrtimer_start і перезапускати за допомогою, наприклад, hrtimer_forward_now, передаючи у якості другого параметру час спрацювання у форматі ktime_t. Для зручності можна скористатися, наприклад, функцією ms_to_ktime.

Allocating Memory
~~~~~~~~~~~~~~~~~

Виділення пам'яті відбувається за допомогою `kmalloc()`

Флаг GFP_KERNEL дозволено працювати як фоновому, так і прямому відновлення.

Обробник переривання повинен швидко відпрацьовувати.
В перериванні велика затримка може призвести до падіння, тому потрібно використовувати `GFP_KERNEL|GFP_ATOMIC`.

Флаг `GFP_KERNEL|GFP_ATOMIC` - виділення без режиму сну.

Результат
---------

.. code-block::

  / # insmod /mnt/src/mdlb2.ko cnt=4 delay=100
  [   433.777411] mdlb2: module: jiffies = 4295101109
  [  433.777411] mdlb2: tasklet: jiffies = 4295101109
  / # rmmod mdlb2
  [  437.060481] mdlb2: module: jiffies= 4295104384
  [  437.060481] mdlb2: timer: array[0] jiffies= 4295101213
  [  437.060481] mdlb2: timer: array[1] jiffies= 4295101317
  [  437.060481] mdlb2: timer: array[2] jiffies= 4295101414
  [  437.060481] mdlb2: timer: array[3] jiffies= 4295101515
  / #

Якщо *cnt* дорівнює 0. Друкує помилку в лог ядра.
Не створює масив і не запускає таймер.

.. code-block::

   / # insmod /mnt/mdlb2.ko cnt=0 delay=100
   [  543.467673] mdlb2: module: jiffies = 4295210812
   [  543.467673] mdlb2: cnt = 0
   [  543.467673] mdlb2: tasklet: jiffies = 4295210812
   / # rmmod mdlb2
   [  546.953115] mdlb2: module: jiffies= 4295214277
   / #

Якщо *delay* дорівнює 0. Висвічуєсть попередження.

.. code-block::

   / # insmod /mnt/mdlb2.ko cnt=4 delay=0
   [  623.792522] mdlb2: module: jiffies = 4295291116
   [  623.795044] mdlb2: tasklet: jiffies = 4295291119
   [  623.796464] mdlb2: delay = 0
   / # rmmod mdlb2
   [  626.391313] mdlb2: module: jiffies= 4295293716
   [  626.393267] mdlb2: timer: array[0] jiffies= 4295291122
   [  626.395469] mdlb2: timer: array[1] jiffies= 4295291122
   [  626.396225] mdlb2: timer: array[2] jiffies= 4295291122
   [  626.398715] mdlb2: timer: array[3] jiffies= 4295291123
   / #

Якщо таймер не порахував *cnt* раз.
Висвічується попередження і друкує в лог ті елементи масиву, які встиг заповнити таймер.

.. code-block::

   / # insmod /mnt/mdlb2.ko cnt=8 delay=1000
   [  691.825320] mdlb2: module: jiffies = 4295359149
   [  691.827093] mdlb2: tasklet: jiffies = 4295359151
   / # rmmod mdlb2
   [  693.715702] mdlb2: module: jiffies= 4295361040
   [  693.717461] mdlb2: timer is queued
   [  693.717970] mdlb2: I killed test_timer
   [  693.719396] mdlb2: timer: array[0] jiffies= 4295360157
   / #

Висновок
--------
Отже в даній роботі я розібрався з *tasket*, *hrtimer* і правильним виділенням пам'яті.
Реалізував модуль який відповідає вимогам завданню.

