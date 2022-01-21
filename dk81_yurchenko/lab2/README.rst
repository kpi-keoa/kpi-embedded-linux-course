Лабораторна робота 2
====================

Завдання
--------
Зробити модуль який приймає 2 аргумента *cnt*, *delay*.
При ініціалізації та розвандаженні модуля виводить *jiffies*.
Модуль запускає tasklet який виводить *jiffies*.
init створую масив розміром *cnt* і запускає таймер з затримкою *delay*.
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

timer
~~~~~
Щоб створити *timer* потрібно
 - створити функцію обробник.
 - Визвати `DECLARE_TASKLET()` або 
    - створити структуру `timer_list`
    - визвати `timer_setup()`
 - визвати `mod_timer()`

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
   [   76.305911] mdlb2: module: jiffies = 4294743551
   [   76.306519] mdlb2: tasklet: jiffies = 4294743552
   / # rmmod mdlb2
   [   79.169453] mdlb2: module: jiffies= 4294746415
   [   79.170112] mdlb2: timer: array[0] jiffies= 4294743656
   [   79.170872] mdlb2: timer: array[1] jiffies= 4294743760
   [   79.171693] mdlb2: timer: array[2] jiffies= 4294743864
   [   79.172483] mdlb2: timer: array[3] jiffies= 4294743968


Якщо *cnt* дорівнює 0. Друкує помилку в лог ядра.
Не створює масив і не запускає таймер.

.. code-block::

   / # insmod /mnt/mdlb2.ko cnt=0 delay=100
   [  139.897877] mdlb2: module: jiffies = 4294807143
   [  139.898555] mdlb2: cnt = 0
   [  139.898973] mdlb2: tasklet: jiffies = 4294807144
   / # rmmod mdlb2
   [  142.241455] mdlb2: module: jiffies= 4294809487

Якщо *delay* дорівнює 0. Висвічуєсть попередження.

.. code-block::

   / # insmod /mnt/mdlb2.ko cnt=4 delay=0
   [  226.913809] mdlb2: module: jiffies = 4294894159
   [  226.914485] mdlb2: tasklet: jiffies = 4294894160
   [  226.915105] mdlb2: delay = 0
   / # rmmod mdlb2
   [  229.993437] mdlb2: module: jiffies= 4294897239
   [  229.994036] mdlb2: timer: array[0] jiffies= 4294894162
   [  229.994758] mdlb2: timer: array[1] jiffies= 4294894164
   [  229.995483] mdlb2: timer: array[2] jiffies= 4294894165
   [  229.996133] mdlb2: timer: array[3] jiffies= 4294894166

Якщо таймер не порахував *cnt* раз.
Висвічується попередження і друкує в лог ті елементи масиву, які встиг заповнити таймер.

.. code-block::

   / # insmod /mnt/mdlb2.ko cnt=4 delay=1000
   [  382.433985] mdlb2: module: jiffies = 4295049679
   [  382.434661] mdlb2: tasklet: jiffies = 4295049680
   / # rmmod mdlb2
   [  384.049609] mdlb2: module: jiffies= 4295051295
   [  384.050147] mdlb2: timer kill
   [  384.050540] mdlb2: timer: array[0] jiffies= 4295050688

Висновок
--------
Отже в даній роботі я розібрався з *tasket*, *timer* і правильним виділенням пам'яті.
Реалізував модуль який відповідає вимогам завданню.

