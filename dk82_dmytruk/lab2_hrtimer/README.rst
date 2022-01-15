Лабораторна робота 2
====================

Завдання
--------
Зробити модуль який приймає 2 аргумента *cnt*, *delay*.
При ініціалізації та розвантаженні модуля виводить *jiffies*.
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
 - створити функцію handler.
 - створити структуру `hrtimer`
 - визвати функцію ініціалізації таймеру `hrtimer_init()`
 - запустити таймер `hrtimer_start()`

Allocating Memory
~~~~~~~~~~~~~~~~~

Виділення пам'яті відбувається за допомогою `kmalloc()`

Флаг GFP_KERNEL дозволяє перервати поточний процес, якщо памяті на даний момент недостатньо для виділення.

Обробник переривання повинен швидко відпрацьовувати.
В перериванні велика затримка може призвести до падіння, тому потрібно використовувати `GFP_KERNEL|GFP_ATOMIC`.

GFP_ATOMIC працює жорсткіше: його використовують в прериваннях, аби виділити память вже і зразу. Пробує виділити як GFP_KERNEL,
але якщо памяті немає, то виділяє із резервних пулів. Процес не призупиняється.

Результат
---------

.. code-block::

   /mnt # insmod lab2.ko cnt=10 delay=100
   [  184.910184] init module>> jiffies = 4294852050
   [  184.910755] tasklet>> jiffies = 4294852051
   /mnt # rmmod lab2.
   [  189.689695] exit module>> jiffies= 4294856830
   [  189.689910] timer >> array[0] jiffies= 4294852152
   [  189.690018] timer >> array[1] jiffies= 4294852252
   [  189.690130] timer >> array[2] jiffies= 4294852353
   [  189.690248] timer >> array[3] jiffies= 4294852453
   [  189.690732] timer >> array[4] jiffies= 4294852556
   [  189.691113] timer >> array[5] jiffies= 4294852652
   [  189.691879] timer >> array[6] jiffies= 4294852752
   [  189.692053] timer >> array[7] jiffies= 4294852852
   [  189.692164] timer >> array[8] jiffies= 4294852951
   [  189.692398] timer >> array[9] jiffies= 4294853052


.. code-block::

   /mnt # insmod lab2.ko cnt=10 delay=1000
   [  278.472147] init module>> jiffies = 4294945612
   [  278.472598] tasklet>> jiffies = 4294945613
   /mnt # rmmod lab2.
   [  303.298704] exit module>> jiffies= 4294970439
   [  303.298983] timer >> array[0] jiffies= 4294946649
   [  303.299114] timer >> array[1] jiffies= 4294947615
   [  303.299245] timer >> array[2] jiffies= 4294948685
   [  303.299539] timer >> array[3] jiffies= 4294949645
   [  303.299667] timer >> array[4] jiffies= 4294950616
   [  303.299769] timer >> array[5] jiffies= 4294951615
   [  303.299976] timer >> array[6] jiffies= 4294952616
   [  303.300128] timer >> array[7] jiffies= 4294953636
   [  303.300424] timer >> array[8] jiffies= 4294954633
   [  303.300679] timer >> array[9] jiffies= 4294955614


Якщо таймер не порахував *cnt* раз.
Висвічується попередження і друкує в лог ті елементи масиву, які встиг заповнити таймер.

.. code-block::

   /mnt # insmod lab2.ko cnt=10 delay=1000
   [  285.138160] init module>> jiffies = 4294952264
   [  285.138416] tasklet>> jiffies = 4294952264
   /mnt # rmmod lab2.
   [  288.246664] exit module>> jiffies= 4294955372
   [  288.246815] exit >> timer is queued
   [  288.247732] exit >> Killed timer_1
   [  288.247915] timer >> array[0] jiffies= 4294953301
   [  288.248042] timer >> array[1] jiffies= 4294954266
   [  288.248150] timer >> array[2] jiffies= 4294955361


Якщо *delay* дорівнює 0. модуль виводить попередження.

.. code-block::

   /mnt # insmod lab2.ko cnt=10 delay=0
   [  347.901874] init module>> jiffies = 4295015027
   [  347.902387] tasklet>> jiffies = 4295015028
   [  347.902554] init>> delay = 0
   [  347.902895] hrtimer: interrupt took 59160 ns
   /mnt # rmmod lab2.
   [  352.752206] exit module>> jiffies= 4295019878
   [  352.752423] timer >> array[0] jiffies= 4295015029
   [  352.752567] timer >> array[1] jiffies= 4295015029
   [  352.752693] timer >> array[2] jiffies= 4295015029
   [  352.752714] timer >> array[3] jiffies= 4295015029
   [  352.752714] timer >> array[4] jiffies= 4295015029
   [  352.752714] timer >> array[5] jiffies= 4295015029
   [  352.752714] timer >> array[6] jiffies= 4295015029
   [  352.754509] timer >> array[7] jiffies= 4295015029
   [  352.754841] timer >> array[8] jiffies= 4295015029
   [  352.754964] timer >> array[9] jiffies= 4295015029


Якщо *cnt* дорівнює 0. Друкує помилку в лог ядра.
Не створює масив і не запускає таймер.

.. code-block::

   /mnt # insmod lab2.ko cnt=0 delay=100
   [   78.060994] init module>> jiffies = 4294745201
   [   78.061216] init>> cnt = 0
   [   78.061427] tasklet>> jiffies = 4294745202
   /mnt # rmmod lab2.
   [   82.309879] exit module>> jiffies= 4294749450


Висновок
--------
Отже в даній роботі я розібрався з *tasket*, *hrtimer* і правильним виділенням пам'яті.
Реалізував модуль який відповідає вимогам завдання.


