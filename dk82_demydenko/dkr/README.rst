*Домашня контрольна робота*.

	*Завдання*:

    *Написати програму, що підпадає вимогам:*

    * Приймає аргументи командного рядка. Для парсингу використати *Argp*.
    * Збирається за допомогою *Make*. *Makefile* містить додаткові цілі *build* і *clean*, де *build* є метою-посиланням на основну, а *clean* – PHONY-метою, що очищає всі артефакти складання.
    * Виводить результат у *stdout*, а помилки – у *stderr*.
    * Містить частину службових функцій, винесених в окремий модуль *utils* (*utils.c* та *utils.h*).
    * Використовує прапорець *--verbose* (*-v*), який змушує програму виводити виконувані дії детально. Інакше вважається, що її висновок використовуватиметься іншою прогамою.
    * Програма приймає довільну кількість координат X Y для точок та виводить координати центральної (рівновіддаленої від інших) точки, а також (якщо вказано прапор *--dist*) відстані до них.

	*Принцип роботи алгоритму пошуку*:
	* При уведенні однієї точки програма завершується
	* При введенні двух точок - за формулою розраховується точка, що знаходиться між ними та відстань до однієї з початкових координат
	* При введенні трьох точок - якщо точки лежать на одній прямій, то виводиться помилка. Якщо лежать на різних прямих (різні координати x,y) то грубо кажучи описується коло, навколо даного трикутника, знаходимо його центр.



	*Результат роботи програми*:

 Результат роботи при введенні однієї точки:

.. code-block:: bash

  [oldem@vm dkr]$ ./main -?
 Usage: main [OPTION...]

  -p, --point=[x,y]          A point
  -v, --verbose              Produce verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message

 Mandatory or optional arguments to long options are also mandatory or optional
 [oldem@vm dkr]$ ./main -p1,4
 Usage: main [OPTION...]
 Try `main --help' or `main --usage' for more information.
 [oldem@vm dkr]$ 


При введенні двух точок:


.. code-block:: bash

 [oldem@vm dkr]$ ./main -?
 Usage: main [OPTION...]

  -p, --point=[x,y]          A point
  -v, --verbose              Produce verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message

 Mandatory or optional arguments to long options are also mandatory or optional
 for any corresponding short options.
 [oldem@vm dkr]$ ./main -p2,1 -p4,7
 O = (3.000000, 4.000000)

При введенні трьох точок:

.. code-block:: bash
 [oldem@vm dkr]$ ./main -?
 Usage: main [OPTION...]

  -p, --point=[x,y]          A point
  -v, --verbose              Produce verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message

 Mandatory or optional arguments to long options are also mandatory or optional
 for any corresponding short options.
 [oldem@vm dkr]$ ./main -p1,2 -p3,4 -p5,7
 O = (14.500000, -9.500000)

*Висновок:*
У результаті виконання даної лабораторної роботи було написано модуль, який створює thread_num потоків, кожен з яких інкрементує значення глобальної змінної glob_var thread_inccnt разів з затрімкою в thread_delay. Без синхронізації працює не зовсім корректно, із власною реалізацією ``Lock()`` та ``unlock()`` працює корректно