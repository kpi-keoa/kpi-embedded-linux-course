#include <pthread.h>
#include <stdio.h>
#include <time.h>

int cnt = 0; 

void *add_func(void *arg)
{	
	(void)arg;
	for (int i = 0; i < 10000000; i++)
		cnt += 1;

	return NULL;
}

int main(void)
{	
	pthread_t thread_1, thread_2;
	
	clock_t start = clock();

	pthread_create(&thread_1, NULL, &add_func, NULL);
	pthread_create(&thread_2, NULL, &add_func, NULL);
	pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);

	clock_t end = clock();

	printf("Sum from the thread %i\n", cnt);
	printf("Execution time %f\n", (double)(end - start) / CLOCKS_PER_SEC);

	return 0;
}

