
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

const int N = 100000000;

int thread_cnt = 0;

void *thread(void *unused);

int main(void)
{
	pthread_t thread_1, thread_2;

	struct timespec start, stop; // for time
	long tt;  


	srand(time(NULL));
	clock_gettime(CLOCK_REALTIME, &start);

  	pthread_create(&thread_1, NULL, thread, NULL);
  	pthread_create(&thread_2, NULL, thread, NULL);
	
	pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);

	clock_gettime(CLOCK_REALTIME, &stop);
	tt=((stop.tv_nsec - start.tv_nsec)/1e3);

	printf("Result of thread: %d\n", thread_cnt);
	
	printf ("time: %ld us\n",tt);

}


void *thread(void *unused)
{
	for (; thread_cnt < N; thread_cnt++);
}
