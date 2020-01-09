#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

void *thread_func ();
long long count1 = 0;
long long incr;

int main (void) 
{
	int res;
	pthread_t a_thread, b_thread;
	//void *pthread_result;

	res = pthread_create (&a_thread, NULL, thread_func, NULL);
	if (res != 0) {
		perror ("Thread A creation failed\n");
		exit (EXIT_FAILURE);
	}	
	else {
		printf ("Thread A created\n");
	}
	
	res = pthread_create (&b_thread, NULL, thread_func, NULL);

	if (res != 0) {
		perror ("Thread B creation failed\n");
		exit (EXIT_FAILURE);
	}	
	else {
		printf ("Thread B created\n");
	}
	
	clock_t t_start = clock();/*
	for (incr = 0; incr < 100000; incr++) {
		count1++;
		//printf ("Main counter up to = %i\n", count1);
		//usleep (1);	
	}*/
	
	res = pthread_join (a_thread, NULL);
	if (res != 0) {
		perror ("Thread A join failed\n");
		exit (EXIT_FAILURE);
	}
	res = pthread_join (b_thread, NULL);
	if (res != 0) {
		perror ("Thread B join failed\n");
		exit (EXIT_FAILURE);
	}
	printf ("Result counter = %i\n", count1);
	clock_t t_stop = clock();
	printf ("Time spended = %i ns.\n", t_stop - t_start);

}

void *thread_func () 
{
	for (incr = 0; incr < 100000000; incr++) {
		count1++;
		//printf ("Thread counter up to = %i\n", count1);
		//usleep (1);
		
	}
}
