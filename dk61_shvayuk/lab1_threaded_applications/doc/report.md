###  Результати виконання лабораторної роботи №1

У ході виконання лабораторної роботи №1 була написана програма на мові С, що інкрементує глобальну змінну із двох потоків. Багатопотоковість реалізована за допомогою бібліотеки pthread.
Вихідний код програми наведений у наступному лістингу:
Результати виконання лабораторної роботи №1

У ході виконання лабораторної роботи №1 була написана програма на мові С, що інкрементує глобальну змінну із двох потоків. Багатопотоковість реалізована за допомогою бібліотеки pthread.
Вихідний код програми наведений у наступному лістингу:
```
	#include <stdbool.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <pthread.h>
	#include <unistd.h>

	int cnt = 0;

	/* This function runs in each thread */
	void *threadfunc(void *args)
	{	
		while (cnt < 10000){
			cnt++;
			usleep (1000);
		}
		return NULL;
	}


	int main (int argc, char *argv[])
	{
		int num_threads = 2;
		
		pthread_t *threads = malloc(num_threads * sizeof(*threads) );
		
		pthread_create(&threads[0], NULL, threadfunc, NULL);
		pthread_create(&threads[1], NULL, threadfunc, NULL);
		
		printf("Waiting for threads...\n:");
		
		pthread_join(threads[0], NULL);
		pthread_join(threads[1], NULL);
		
		printf ("Counter value: %i\n", cnt);
		
		return 0;
	}
```

Збірка програми виконується за допомогою системи збірки Make. Makefile має дві функції: видалення попеедніх версій бінарних файлів програми та, власне, збірка програми. Його вихідний код наведено нижче.
```
all:
	clear; clear
	rm -rfv build
	mkdir build
	gcc src/example_mt.c -O0 -o build/example_mt_O0 -Wall -Wpedantic -pthread
	gcc src/example_mt.c -O2 -o build/example_mt_O2 -Wall -Wpedantic -pthread
clean:
	rm -rfv build
	

```

Результат виконання програми на платформах х86 та ARM виявився ідентичним: програма щавжди показувала правильний результат.
Результат виконання на платформі х86:
```
[max@hpprobook build]$ ./example_mt_O0
Waiting for threads...
:Counter value: 10000
[max@hpprobook build]$ ./example_mt_O2
Waiting for threads...
:Counter value: 10000
```
Результат виконання на платформі ARM:
```
debian@beaglebone:~/lab1_threaded_applications/build$ ./example_mt_O0
Waiting for threads...
:Counter value: 100
debian@beaglebone:~/lab1_threaded_applications/build$ ./example_mt_O2
Waiting for threads...
:Counter value: 100
```
