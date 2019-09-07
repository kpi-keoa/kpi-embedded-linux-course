Звіт з виконання лабораторної роботи №1  
=======================================

1. Код
------

Під час виконання даної лабораторної роботи була опрацьована та проаналізована 
реалізація багатопоточності на різних архітектурах, а саме x86(PC) та ARM(Beagle 
Board). Для цього була написана програма на мові С з використанням бібліотеки 
lpthread та Makefile для її збірки та запуску. В програмі запущено два потоки, 
кожен з яких інкрементує одну й ту саму глобальну змінну. Лістинг програми 
наведено нижче:

.. code-block::

	#include <pthread.h>
	#include <stdio.h>
	#include <time.h>

	int cnt = 0; 

	void *add_func()
	{	
		int i;
		
		for (i = 0; i < 10000000; i++)
			cnt += 1;

		return NULL;
	}

	int main(void)
	{	
		pthread_t thread_1, thread_2;
		
		clock_t start = clock();

		pthread_create(&thread_1, NULL, add_func, NULL);
		pthread_create(&thread_2, NULL, add_func, NULL);
		pthread_join(thread_1, NULL);
		pthread_join(thread_2, NULL);

		clock_t end = clock();

		printf("Sum from the thread %i\n", cnt);
		printf("Execution time %f\n", (double)(end - start) / CLOCKS_PER_SEC);

		return 0;
	}

2. Результат
------------

Програма була скомпільована з прапорцями оптимізації O0 та O2 та запущена на 
кожній з архітектур. 
Результат роботи на архітектурі x86:

.. code-block::

  [maksimo0@maksimo0-pc lab1_first_embedded_app]$ make all
  gcc -o main_1 -O0 -Wall -Wextra -Wpedantic -lpthread thread.c
  gcc -o main_2 -O2 -Wall -Wextra -Wpedantic -lpthread thread.c
  ./main_1
  Sum from the thread 13267686
  Execution time 0.083206
  ./main_2
  Sum from the thread 20000000
  Execution time 0.000294
  rm -rf main_1 main_2

Результат роботи на архітектурі ARM:

.. code-block::

  debian@beagleboard:~/build$ make all
  gcc -o main_1 -O0 -Wall -Wextra -Wpedantic -lpthread thread.c
  gcc -o main_2 -O2 -Wall -Wextra -Wpedantic -lpthread thread.c
  ./main_1
  Sum from the thread 12547782
  Execution time 2.4946
  ./main_2
  Sum from the thread 20000000
  Execution time 0.001404
  rm -rf main_1 main_2
