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

 / # insmod /mnt/khmod.ko mood=1
 [   11.694347] khmod: loading out-of-tree module taints kernel.
 [   11.711966] Registered device with 248:0
 [   11.739466] insmod (96) used greatest stack depth: 13880 bytes left
 / # cd /dev && mknod khmod c 248 0 && mdev -s
 /dev # cat khmod
 Did I ever tell you what insanity is?
 This is when you come here for the 0 time
 and think that something will change
 /dev # cat khmod
 Did I ever tell you what insanity is?
 This is when you come here for the 1 time
 and think that something will change
 /dev # /mnt/./ioc 4
 [   19.910191] Now mood is - 4
 [   19.930100] ioc (102) used greatest stack depth: 13208 bytes left
 /dev # cat khmod
 The foundation of wisdom is patience. 3
 /dev # cat khmodcat khmod
 cat: can't open 'khmodcat': No such file or directory
 The foundation of wisdom is patience. 4
 /dev # rmmod khmod

Висновки
~~~~~~~~
Було зібрано та протестовано власний модуль ядра. Під час виконання, навчився створювати символьні девайси. Крім цього вияснив як відбувається взаємодія драйверів з юзерспейсом.
