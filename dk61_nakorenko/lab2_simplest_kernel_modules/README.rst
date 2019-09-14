Звіт до лабораторної роботи №2 
~~~~~~~~~~
Завдання:
~~~~~~~~~

#. Перенести на плату й встановити заголовки ядра.
#. Зібрати модуль ядра на **х86** та **ВВХМ**, після кожного підключення модуля до ядра дивитися логи через dmesg.
#. Взяти секундомір. Засікти час між двома включеннями модуля на x86 і BBXM. Взяти відповідні значення jiffies з логів. 
   Відповісти на питання: чи залежить різниця в двох значеннях jiffies від платформи?
#. Експериментально розрахувати час (в мілісекундах), рівне 1 jiffies. Порівняти з теоретичним значенням. 
   Результати включити в протокол по роботі.
#. Виправити код з прикладу ``firstmod.c``, щоб замість username виводилось ім'я передане в якості аргумента.
#. Додати в ``init`` модуля тасклет, який також виведе значння jiffies.
   Пояснити різницю між результатами виводів в ``init`` і тасклеті.
   
Виконання завдань
~~~~~~~~~~~
1. Встановлення заголовків
--------------
Для того, щоб встановити заголовки, їх потрібно спочатку завантажити. Для цього потрібно перейти за посиланням ``http://repos.rcn-ee.com/debian/``
далі в вікні шляху запиту додаємо ``pool/main/l/linux-upstream/linux-headers-4.18.11-armv7-x11_1stretch_armhf.deb``, щоб запит мав вигляд
``http://repos.rcn-ee.com/debian/pool/main/l/linux-upstream/linux-headers-4.18.11-armv7-x11_1stretch_armhf.deb``. Після завантаження,
цей файл потрібно переслати на плату, я це зробив за допомогою ``sshfs``. На платі розпакував щойно переданий архів командою ``sudo dpkg -i ./linux-headers-4.18.11-armv7-x11_1stretch_armhf.deb``.

2. Збирання модуля ядра
-------------
Щоб зібрати модуль ядра запуститв Makefile з прикладу, він зтворив з файлу firstmod.c, що також був в прикладі, файл firstmod.ko.
Цей файл і є файлом модуля ядра. Тепер потрібно завантажити цей файл командою ``insmod firstmod.ko``. Щоб побачити логи ядра використовують
``dmesg -k``.

 .. image:: img/1.png



3. Визначення часу між двома включеннями на різних архітектурах
----------------

Через деякі технічні проблеми в мене не вийшло провести маніпуляції з платою в цьому та 4-му завданні, тому поки я їх пропустив, щоб виконати повністью в найближчий час. 

4. Експерементальний розрахунок
---------------

*Буде доповнено пізніше*


5. Передача аргументу в код модуля ядра
--------------

Модулі можуть приймати аргументи командного рядка, але не через ``argc / argv``.
Щоб дозволити передачу аргументів у модуль, потрібно оголосити змінні які приймуть значення аргументів командного рядка як глобальні,
а потім використати макрос ``module_param ()``, (визначений у ``linux / moduleparam.h``). 
Потім під час виконання insmod потрібно заповнити змінні аргументами командного рядка, 
наприклад ``./insmod mymodule.ko myvariable = 5.``

В моєму випадку, потрібно передати в програму рядок, тому змінну оголошено як ``static char *name = "name"``.
Макрос ``module_param()`` визначений як ``module_param (name, charp, 0000);``.
Щоб програма виводила цей аргумент, потрібно в функції ``__init firstmod_init`` додати специфікатор ``%s`` та ``name``. 

  .. image:: img/hellora7e.png

6. Додавання тасклету
--------------

Тасклет - достатньо складна річ, яка виконує відкладену обробку переривань. Спочатку потрібно задекларувати тасклет.
Для цього викликаємо ``DECLARE_TASKLET(tasklet, func, 0)``, де ``tasklet``- ім'я тасклету, ``func`` - функція-обробник,
``0``- аргумент, що передається в функцію обробник. Щоб вивести значення *jiffies* за дапомогою тасклету, я в функції ``func``
за допомогою ``printk`` вивожу аргумент ,що передається в ``func``. Потім, в функції  ``__init firstmod_init`` викликаю ``func``
передаючи ``jiffies`` як аргумент.

  .. image:: img/tasklet.png

Вихідний код програми:
 
 .. code-block:: C
 
  #include <linux/module.h>	// required by all modules
  #include <linux/kernel.h>	// required for sysinfo
  #include <linux/init.h>		// used by module_init, module_exit macros
  #include <linux/jiffies.h>	// where jiffies and its helpers reside
  #include <linux/interrupt.h>

  MODULE_DESCRIPTION("Basic module demo: init, deinit, printk, jiffies");
  MODULE_AUTHOR("thodnev");
  MODULE_VERSION("0.1");
  MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

  static char *name = "name";

  module_param (name, charp, 0000);

  void func(unsigned long arg)
  {
    printk(KERN_INFO "Tasklet jiffies = %lu\n", arg);
  }

  DECLARE_TASKLET(tasklet, func, 0);

  static int __init firstmod_init(void)
  {
    printk(KERN_INFO "Hello, %s\njiffies = %lu\n", name, jiffies);

    func(jiffies);

    return 0;
  }

  static void __exit firstmod_exit(void)
  {
    printk(KERN_INFO "Long live the Kernel!\n");
  }

  module_init(firstmod_init);
  module_exit(firstmod_exit);

