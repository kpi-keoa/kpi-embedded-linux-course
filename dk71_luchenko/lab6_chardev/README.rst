==========================================================
**Лабораторна робота №6. символьний пристрій**
==========================================================


**Завдання:**
~~~~~~~~~~~~~
   
— Написати свій символьний пристрій з ioctl.
      

**Хід роботи:**
~~~~~~~~~~~~~~~
Код написано на базі лекційного матеріалу по символьних пристроях. Сам модуль рахує кількість читань файлу, та виводить повідомлення у вибраному форматі. Вибір формату здійснюється за допомогою ioctl. Для тестування працездатності було написано просту юзерспейсну програму. 

Тест модуля:

.. code-block:: bash

 / # insmod /mnt/mymod.ko mood=1
 [   11.694347] mymod: loading out-of-tree module taints kernel.
 [   11.711966] Registered device with 248:0
 [   11.739466] insmod (96) used greatest stack depth: 13880 bytes left
 / # cd /dev && mknod mymod c 248 0 && mdev -s
 /dev # cat mymod
 Oh, is it you again? Already 0 times
 /dev # cat mymod
 Oh, is it you again? Already 1 times
 /dev # /mnt/./ioc 3
 [   17.630126] Now mood is - 3	
 [   17.690103] ioc (102) used greatest stack depth: 13208 bytes left
 /dev # cat mymod
 Do. Or do not. There is no try. — Yoda said it 3 times
 /dev # cat mymodcat mymod
 cat: can't open 'mymodcat': No such file or directory
 Do. Or do not. There is no try. — Yoda said it 4 times
 /dev # rmmod mymod

Висновки
~~~~~~~~
Було зібрано та протестовано власний модуль ядра. Під час виконання, навчився створювати символьні девайси. Крім цього вияснив як відбувається взаємодія драйверів з юзерспейсом. Якби не тільт можна було б і раніше здати
