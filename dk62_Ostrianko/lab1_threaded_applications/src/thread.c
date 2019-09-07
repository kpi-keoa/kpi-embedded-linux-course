#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include "../inc/header.h"


int main()
{
	pthread_t first_thread, second_thread;
	
	clock_t start = clock();

	pthread_create(&first_thread, NULL, func, NULL);
	pthread_create(&second_thread, NULL, func, NULL);
	pthread_join(first_thread, NULL);
	pthread_join(second_thread, NULL);
	
	clock_t end = clock();

	printf("Program counted to %i\n", global_counter);
	printf("Program ended for %f seconds.\n", (
	double)(end - start) / CLOCKS_PER_SEC);

	return 0;	
}
