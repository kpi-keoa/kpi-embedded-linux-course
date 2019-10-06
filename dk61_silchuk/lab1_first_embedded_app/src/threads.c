#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define SUCCESS_STATUS 0

const int threads_amt = 2;			
const long long cnt_end = 100000000;
long long counter = 0;				

void *thread_counter(void *arg)
{
	for (long long i = 0; i < cnt_end; i++) {
		counter++;
	}
}

int main(void)
{
	int thrd_status;

	pthread_t threads[threads_amt];

	printf("Creating %d threads.\n", threads_amt);

	for (int i = 0; i < threads_amt; i++) {
		thrd_status = pthread_create(&threads[i], NULL, &thread_counter, NULL);

		if (thrd_status != SUCCESS_STATUS) {
			printf("Thread creating error. Status = %d.\n", thrd_status);
			exit(EXIT_FAILURE);	
		}
	}

	printf("%d threads created successfully.\n", threads_amt);

	for (int i = 0; i < threads_amt; i++) {
		thrd_status = pthread_join(threads[i], NULL);

		if (thrd_status != SUCCESS_STATUS) {
			printf("Thread joining error. Status = %d.\n", thrd_status);
			exit(EXIT_FAILURE);	
		}
	}

	printf("Counter value: %lld.\n", counter);

	return 0;
}

