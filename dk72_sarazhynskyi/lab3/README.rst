=================================================
**Лабораторна робота №3 засоби відкладеної роботи: таймер високої точності**
=================================================

Завдання
---------------------------------------------------------

* ознайомитись hrtimer
* на основі модулю з минулої лабораторної роботи створити модуль що буде мати такий самий
функціонал але замість звичайного таймеру будемо використовувати таймер високої роздільної здатності

Хід роботи
----------------------------------------------------------

**Написання модуля ядра**
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Згідно з отриманим завданням було створено модуль ядра що є "апгрейдом" модуля ядра з минулої лабораторної роботи, де замість використаного в минулій роботі timer wheel було використано HR timer.

У зв'язку з завданням потрібно використовувати HR timer, по цій причині було створено змінну  ktime для виставлення значення expire, та звичайно замінено оброрбник переривань таймеру (обробник HR таймеру повинен повертати змінну типу (enum) HRTIMER_RESTART або HRTIMER_NORESTART).
При поверненні значення HRTIMER_RESTART таймер запускається повторно після завершення роботи обробника, в іншому випадку таймер повторно не активується. Для зазначеня часу відпрацювання таймеру була використана функція
``hrtimer_forward`` затримка задається за допомогую змінної типу данних ktime_t, для переводу секунд до цього типу данних використовуємо функцію ktime_set(). Видалення HR таймеру відбувається за допомогою функції ``hrtimer_cancel``

Тести та демонстрація роботи hrtimer в порівняні Timeout timer:

Timeout_timer:
 
.. code-block:: bash

 / # insmod mnt/mymod.ko cnt=10 delay=5
 [  102.881771] Init>> jiffies is 4294770209
 [  102.882619] tasklet>> jiffies = 4294770210
 / # rmmod mymod
 [  134.912584] exit>> tim_cnt 10
 [  134.912584] exit>> countrer number 0 has the 4294770217 jiffies val
 [  134.913821] exit>> countrer number 1 has the 4294770223 jiffies val
 [  134.914597] exit>> countrer number 2 has the 4294770232 jiffies val
 [  134.915331] exit>> countrer number 3 has the 4294770238 jiffies val
 [  134.916079] exit>> countrer number 4 has the 4294770245 jiffies val
 [  134.916778] exit>> countrer number 5 has the 4294770251 jiffies val
 [  134.917492] exit>> countrer number 6 has the 4294770258 jiffies val
 [  134.918191] exit>> countrer number 7 has the 4294770264 jiffies val
 [  134.918922] exit>> countrer number 8 has the 4294770270 jiffies val
 [  134.919457] exit>> countrer number 9 has the 4294770277 jiffies val
 [  134.920226] exit>> dealoc: array is deallocated
 [  134.920925] exit>> god save the Kernel!; jiffies = 4294802249
 
hrtimer:
 
.. code-block:: bash

 / # insmod /mnt/mymod.ko cnt=10 delay=5
 [  105.143768] mymod: loading out-of-tree module taints kernel.
 [  105.157489] Init>> jiffies is 4294772218
 [  105.159567] tasklet>> jiffies = 4294772220
 [  105.165681] insmod (79) used greatest stack depth: 13784 bytes left
 / # rmmod mymod.ko
 [  113.690198] exit>> tim_cnt: 10
 [  113.690198] exit>> countrer number 0 has the 4294772226 jiffies val
 [  113.690198] exit>> countrer number 1 has the 4294772231 jiffies val
 [  113.690198] exit>> countrer number 2 has the 4294772238 jiffies val
 [  113.690198] exit>> countrer number 3 has the 4294772241 jiffies val
 [  113.690198] exit>> countrer number 4 has the 4294772246 jiffies val
 [  113.690198] exit>> countrer number 5 has the 4294772251 jiffies val
 [  113.690198] exit>> countrer number 6 has the 4294772256 jiffies val
 [  113.690198] exit>> countrer number 7 has the 4294772261 jiffies val
 [  113.690198] exit>> countrer number 8 has the 4294772266 jiffies val
 [  113.690198] exit>> countrer number 9 has the 4294772271 jiffies val
 [  113.690198] exit>> dealoc: array is deallocated
 [  113.690198] exit>> god save the Kernel!; jiffies = 4294780750

Висновки
------------
В результаті роботи було перероблено свій модуль ядра з лабораторнрої роботи №2 з заміною таймеру на hr таймер. Таймер високої роздільної здатності має набагато більшу точність, це зумовлено тим що роздільна здатність hr таймерів може бути порядку наносекунд а в  timer wheel
мінімально можлива роздільна здатність - 3ms така велика різниця в точності викликана призначенням цих таймерів timer wheel використовуються на приклад для визначення таймаутів очікування а таймери високої роздільної здатності використовують наприклад для генерації ШІМ сигналу. Потрібно зазначити що незважаючи на точність hr таймерів іноді виникали похибки в 1-2 jiffies, можливо це пов'язано з завантаженістю ядра іншими процесами.
