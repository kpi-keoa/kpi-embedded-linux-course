#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>


void *thread_func(void *unused_args);

long long count;

int main(void)
{
	
	int res;
	pthread_t a_thread;
	
	void *pthread_result;

	res = pthread_create(&a_thread, NULL, thread_func, NULL);

		if (res != 0)  {
			perror("Thread creation failed\n");
			exit(EXIT_FAILURE);
		}	
		
		else  {
			printf("Thread created\n");
		}
	
	clock_t t_start1 = clock();
		
		while (count < 100000000) {
		count++;		
		}

	printf("count1 = %i\n", count);
	clock_t t_stop1 = clock();
	printf("time 1 = %i us.\n", t_stop1-t_start1);

	res = pthread_join (a_thread, NULL);
		
		if (res != 0)  {
		perror("Thread join failed\n");
		exit(EXIT_FAILURE);
		}	
}

void *thread_func(void *unused_args) 
{	
	clock_t t_start2 = clock();
	
		while (count < 100000000) {
			count++;
		}
	
	printf("count2 = %i\n", count);
	clock_t t_stop2 = clock();
	printf("time 2 = %i us.\n", t_stop2 - t_start2);

}
