Лабораторна робота 2
====================

Завдання
--------
Зробити модуль який приймає 2 аргумента *cnt*, *delay*.
При ініціалізації та розвандаженні модуля виводить *jiffies*.
Модуль запускає tasklet який виводить *jiffies*.
init створую масив розміром *cnt* і запускає таймер з затримкою *delay* ms.
Timer записує *jiffies* в масив *cnt* раз.
При розвантаженні модуля масив потрібно надрукувати в лог.

Xiд роботи
----------

tasket
~~~~~~
Щоб стровити tasket потрібно
 - створити функцію обробник.
 - Визвати `DECLARE_TASKLET()`
 - Визвати `tasklet_schedule()`

hrtimer
~~~~~~~
Щоб створити *hrtimer* потрібно
 - створити функцію обробник.
 - створити структуру `hrtimer`
 - Визвати `hrtimer_init()`
 - визвати `hrtimer_start()`
    тут потрібна функція ms_to_ktime() яка перетворює число в ms в ktime_t формат

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

   / # insmod /mnt/mdlb2.ko cnt=4 delay=100
   [   15.970997] mdlb2: loading out-of-tree module taints kernel.
   [   15.972034] mdlb2: module: jiffies = 4294683218
   [   15.972665] mdlb2: tasklet: jiffies = 4294683219
   [   15.973354] insmod (98) used greatest stack depth: 13568 bytes left
   / # rmmod mdlb2
   [   27.001522] mdlb2: module: jiffies= 4294694248
   [   27.002082] mdlb2: timer: array[0] jiffies= 4294683320
   [   27.002794] mdlb2: timer: array[1] jiffies= 4294683420
   [   27.003504] mdlb2: timer: array[2] jiffies= 4294683520
   [   27.004121] mdlb2: timer: array[3] jiffies= 4294683620
   / #

Якщо *cnt* дорівнює 0. Друкує помилку в лог ядра.
Не створює масив і не запускає таймер.

.. code-block::

   / # insmod /mnt/mdlb2.ko cnt=0 delay=100
   [  141.674015] mdlb2: module: jiffies = 4294808920
   [  141.674669] mdlb2: cnt = 0
   [  141.675014] mdlb2: tasklet: jiffies = 4294808921
   / # rmmod mdlb2
   [  157.353589] mdlb2: module: jiffies= 4294824600
   / #

Якщо *delay* дорівнює 0. Висвічуєсть попередження.

.. code-block::

   / # insmod /mnt/mdlb2.ko cnt=4 delay=0
   [  202.057958] mdlb2: module: jiffies = 4294869304
   [  202.058629] mdlb2: tasklet: jiffies = 4294869305
   [  202.059270] mdlb2: delay = 0
   [  202.059728] hrtimer: interrupt took 33450 ns
   / # rmmod mdlb2
   [  259.305588] mdlb2: module: jiffies= 4294926552
   [  259.306198] mdlb2: timer: array[0] jiffies= 4294869306
   [  259.306849] mdlb2: timer: array[1] jiffies= 4294869306
   [  259.307574] mdlb2: timer: array[2] jiffies= 4294869306
   [  259.308271] mdlb2: timer: array[3] jiffies= 4294869307
   / #

Якщо таймер не порахував *cnt* раз.
Висвічується попередження і друкує в лог ті елементи масиву, які встиг заповнити таймер.

.. code-block::

   / # insmod /mnt/mdlb2.ko cnt=8 delay=1000
   [  313.210030] mdlb2: module: jiffies = 4294980456
   [  313.210811] mdlb2: tasklet: jiffies = 4294980457
   / # rmmod mdlb2
   [  315.457493] mdlb2: module: jiffies= 4294982704
   [  315.457886] mdlb2: timer is queued
   [  315.458267] mdlb2: I killed test_timer
   [  315.458817] mdlb2: timer: array[0] jiffies= 4294981458
   [  315.459588] mdlb2: timer: array[1] jiffies= 4294982458
   / #

Висновок
--------
Отже в даній роботі я розібрався з *tasket*, *hrtimer* і правильним виділенням пам'яті.
Реалізував модуль який відповідає вимогам завданню.

