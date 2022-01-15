Лабораторна робота 0
======================

Завдання
--------

Написату просту гру на мові С.


Хід роботи
----------

Було написано код гри на мові С. Сенс гри в тому, що ми вводимо число, після чого комп'ютер генерує ще одне рандомне число. Два числа порівнюються і результатом є виведення на екран того, хто переміг: комп'ютер або ім'я гравця, яке ми ввели.


Результат
---------
.. code-block::

	User@WIN-L4I4C2H61UO MINGW64 /c/Users/User/Desktop/lab0
	$ make
	gcc -c lab0.c -std=gnu11 -Wall
	gcc -std=gnu11 -Wall -o lab0 lab0.o
	User@WIN-L4I4C2H61UO MINGW64 /c/Users/User/Desktop/lab0 ./lab0 
	Enter your Name: ff
	Hello, ff !
	Enter letter "n" to play or another symbol to exit
	n

	ff win!
	Enter letter "n" to play or another symbol to exit
	n

	ff win!
	Enter letter "n" to play or another symbol to exit
	n

	Computer win!
	Enter letter "n" to play or another symbol to exit
	n

	Computer win!
	Enter letter "n" to play or another symbol to exit
	n

	ff win!
	Enter letter "n" to play or another symbol to exit
	d


Висновок
~~~~~~~~

При виконанні лабораторної роботи був написаний код для простої гри на мові С.

