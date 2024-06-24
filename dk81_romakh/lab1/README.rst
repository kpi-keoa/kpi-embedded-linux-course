Лабораторна робота 1
====================

Завдання:
---------

1. Зібрати Linux ядро з використанням BusyBox;
2. Запустити його на эмуляторі QEMU;
3. Створити модуль ядра, щоб після завершення модуль друкував в лог поточне значення jiffies і час в секундах, який пройшов від init до exit. 

Виконання роботи:
-----------------

Збірка Linux-ядра
~~~~~~~~~~~~~~~~~
Спочатку необхідно встановити QEMU ``sudo pacman -S base-devel qemu qemu-arch-extra bc cpio``.

Перед зборкою ядра сконфігурувати його за допомогою ``make menuconfig``.
Запустити зборку ядра ``make -j5 vmlinux``, для 4 ядер.
Для ядра необхідно додатково зібрати bzImage ``make -j5 bzImage``.

Зібрати BusyBox
~~~~~~~~~~~~~~~
BusyBox - це мінімалістичний набір базових команд реалізований у вигляді одного виконуваного файлу.

.. code-block::
	make -j5 CRYPT_AVAILABLE=n install

Після чого необхідно перейти в папку *_install* та створити файл папки */proc* */sys* і надати їм необхідні права доступу:

.. code-block::
	cd _install
	mkdir proc
	mkdir sys
	chmod 555 proc
	chmod 555 sys

Також необхідно створити файл */init*б він має мати права для виконання. Ядро виконає цей файл одразу ж після першочергових етапів запуску.

Створити з цієї диреткорії архів ``find . -print0 | cpio --null --owner=0:0 -ov --format=newc | gzip -9 > ../../initramfs.cpio.gz``


Збірка модуля
~~~~~~~~~~~~~

Для того щоб зібрати модуль я створив *Makefile*, в директорії з мейкфайлом та вихідним файлом необхідно виконати ``make modules`` 


Запуск ядра
~~~~~~~~~~~
Запуск відбувається на віртуальній машині QEMU 
.. code-block::
	qemu-system-x86_64 -m 256M -smp 4 -kernel "./arch/x86/boot/bzImage" -initrd "./initramfs.cpio.gz" \                             -append "console=ttyS0" -nographic \
                   -drive file=fat:rw:../Documents/Labs/Lab1,format=raw,media=disk



Робота з модулем ядра
~~~~~~~~~~~~~~~~~~~~~

Створемо папку ``mkdir /mnt``
Змонтуємо перший розділ диска``mount -t vfat /dev/sda1 /mnt``
Завантажемо зібраний модуль:
 
.. code-block::
	insmod /mnt/firstmod.ko username="VitaliiR"
	#вивід
	#[  668.002165] Hello, VitaliiR!
	#[  668.002165] jiffies = 4295335315


Перевіремо список завантажених модулей 
.. code-block::
	lsmod
	#вивід
	#firstmod 16384 0 - Live 0xffffffffc029a000 (O)

Вигрузити модуль ``rmmod firstmod``
	.. code-block::
	rmmod firstmod
	#вивід
	#[  674.202917] Long live the Kernel!
	#[  674.202917] Time from init to exit = 6sec


Висновок
--------

Під час виконання лабораторної роботи я навчився збирати Linux ядро.
Запускати його на віртуальній машині та завантажувати власні модулі.
Запуск на віртуальній машині використовують для запровадження безпечного тестування модуля.