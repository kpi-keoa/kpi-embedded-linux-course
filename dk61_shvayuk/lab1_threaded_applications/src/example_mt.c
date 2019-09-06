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
