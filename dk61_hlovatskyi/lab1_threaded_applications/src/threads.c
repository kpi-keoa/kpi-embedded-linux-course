#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int const NUM_OF_THREADS = 2;
long long NUM_LOOPS = 100000000; //not a const
// obviously a huge number

long long sum = 0; // global var that contains final result

void *counting_thread(void *arg)
{
	int offset = *(int *)arg;
	for (; NUM_LOOPS > 0; NUM_LOOPS--) {
		sum += offset;	
	} 
	pthread_exit(NULL);
}

int main(void)
{
	pthread_t id[NUM_OF_THREADS];   
	
	int offset = 1; // our thread_arg
	int res = 0;
	for (int i = 0; i < NUM_OF_THREADS; i++) {	
		res = pthread_create(&id[i], NULL, &counting_thread, &offset);
		if (0 != res) {
			perror("Thread creation failed\n");
			exit(EXIT_FAILURE);
		}
	}

	printf("Waiting for thread to finish...\n");
	for (int i = 0; i < NUM_OF_THREADS; i++) {	
		res = pthread_join(id[i], NULL);
		if (0 != res) {
			perror("Thread join failed\n");
			exit(EXIT_FAILURE);
		}
	}
	printf("Sum = %lld\n", sum);
	return 0;
}

