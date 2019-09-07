
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define N 100000000

int stream_cnt;

void *stream();

int main()
{
	pthread_t thread_1, thread_2;

	struct timespec start, stop; // for time
	long int tt;  


	srand(time(NULL));
	clock_gettime (CLOCK_REALTIME, &start);

  	pthread_create(&thread_1,NULL,stream, NULL);
  	pthread_create(&thread_2,NULL,stream, NULL);
	
	pthread_join(thread_1,NULL);
	pthread_join(thread_2,NULL);

	clock_gettime (CLOCK_REALTIME, &stop);
	tt=((stop.tv_nsec - start.tv_nsec)/1e3);

	printf("Result of stream: %d\n", stream_cnt );
	
	printf ("time: %ld us\n",tt);

}


void *stream() 
{
	for (stream_cnt = 0; stream_cnt < N; stream_cnt++);

}