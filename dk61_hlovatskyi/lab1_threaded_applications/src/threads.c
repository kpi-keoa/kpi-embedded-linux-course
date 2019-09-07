#include <stdio.h>
#include <pthread.h>

int const NUM_OF_THREADS = 2;
long long NUM_LOOPS = 100000000;
// obviously a huge number

long long sum = 0; // global var that contains final result

void *counting_thread(void *arg)
{
	int offset = *(int *) arg;
	for (; NUM_LOOPS > 0; NUM_LOOPS--) {
		sum += offset;	
	} 
	pthread_exit(NULL);
}

int main(void)
{
	pthread_t id[NUM_OF_THREADS];   
	
	int offset = 1; // our var for args
	
	for (int i = 0; i < NUM_OF_THREADS; i++) {	
		pthread_create(&id[i], NULL, &counting_thread, &offset);
	}

	for (int i = 0; i < NUM_OF_THREADS; i++) {	
		pthread_join(id[i], NULL);
	}
	
	printf("Sum = %lld\n", sum);
	return 0;
}

