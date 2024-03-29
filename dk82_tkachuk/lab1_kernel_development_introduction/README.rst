 ==========================================================
**Лабораторна робота №1 Введення в розробку модулів ядра**
==========================================================

**Завдання:**
~~~~~~~~~~~~~
* Завантажити і сконфігурувати ядро.
* Зібрати ядро.
* Завантажити і зібрати BusyBox. Утворити Initramfs.
* Завантажити зібране ядро з образом Initramfs.
* Зібрати перший модуль ядра.
* Модифікувати перший модуль: замість $username повинне виводитись ім'я користувача, передане в якості параметра.
* Якщо параметр не заданий - вивести "$username" і в лог ядра вивести повідомлення з рівнем логінгу WARNING, що ім'я не задане.
* Доповнити exit_callback модуля, щоб він виводив значення jiffies та час в секундах від init до exit.

~~~~~~~~~~~~~~~
**Хід роботи:**

Як приклад був використаний навчальний модуль ``firstmod``.

**Використані бібліотеки**
--------------------------

* ``<linux/module.h>`` потрібна для всіх модулів;
* ``<linux/kernel.h>`` використовується для *sysinfo*;
* ``<linux/init.h>`` використовується для ініціалізації та закриття;
* ``<linux/jiffies.h>`` опис jiffies та допоміжні функції.

**Використані макроси та функції**
----------------------------------

* ``MODULE_DESCRIPTION`` опис модуля;

* ``MODULE_AUTHOR`` автори модуля;

* ``MODULE_VERSION`` версія модуля;

* ``MODULE_LICENSE`` тип ліцензії. Використовується подвійна: MIT/GPL;

* ``module_param`` необхідна опису правил передачі параметрів в модуль;

* ``jiffies_delta_to_msecs`` використовується для розрахунку проміжку часу. На вхід очікує різницю *jiffies*;

* ``printk`` використовується для виведення інформації в консоль.

Висновки
~~~~~~~~

Було зібрано ядро linux з мінімальним набором оточення з використанням BusyBox, початкового образу Initramfs. Емуляція поводилася з використанням QEMU. Нижче показано результати роботи модуля з передачею параметра ``user_name`` та без. При передачі без параметра, виводиться відповідне сповіщення в лог ядра з рівнем ``KERN_WARNING`` та ім'ям користувача - ``$username``. При вивантаженні модуля виводиться час його роботи в секундах.


.. code-block:: bash

 Без параметрів:
 / # insmod /mnt/mod_for_lab1.ko
 [   68.762555] mod_for_lab1: loading out-of-tree module taints kernel.
 [   68.769621] Module: init module... t:4294736082
 [   68.772929] Username not set!
 [   68.772957] Hello, $username!
 [   68.772957]  Jiffies = 4294736083
 [   68.775772] insmod (93) used greatest stack depth: 13688 bytes left
 / # rmmod mod_for_lab1
 [   85.939720] Long live the Kernel! Work time - 17 sec.

 З параметром:
 / # insmod /mnt/mod_for_lab1.ko user_name="111111"
 [   97.361531] Module: init module... t:4294764676
 [   97.361531] Hello, 111111!
 [   97.361531]  Jiffies = 4294764676
 / # rmmod mod_for_lab1
 [  109.695845] Long live the Kernel! Work time - 12 sec.
