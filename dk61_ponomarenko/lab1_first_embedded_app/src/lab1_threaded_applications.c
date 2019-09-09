#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

const int THREADS_MAX_AMOUNT = 2;

int thread_cnt = 0, loop_length = 0;

void *thread(void *argument)
{
	loop_length = *(int*)argument;
	while (thread_cnt++ < loop_length);
        return NULL;
}

int main(int argc, char **argv)
{	
	/* Checking if user entered data in console correctly */
        if (argc != 3) {
		printf("Error. Invalid number of arguments. Enter again");
		return -1;
        }
        
        /* Convert entered string from console to integer */
        loop_length = atoi(argv[1]);	
        
        /* Initialize descryptor of thread */
        pthread_t threads[THREADS_MAX_AMOUNT];
	
	clock_t begin_count = clock();
	
	printf("Created two threads\n");
        
        /* Creating constant quantity of threads, defined inb4 */
        for (int i = 0; i < THREADS_MAX_AMOUNT; i++) {
                pthread_create(&threads[i], NULL, thread, &loop_length);
        }
        
        printf("Waiting for threads to finish execution...\n");
	
	/* Waiting for the end of threads */
        for (int j = 0; j < THREADS_MAX_AMOUNT; j++) {
		pthread_join(threads[j], NULL);	
        }
	
	clock_t end_count = clock();
	
	/* Count time, spent for executing thread from the start to the end*/
	double exec_time = (double)(end_count - begin_count) / CLOCKS_PER_SEC;
	
        printf("Threads have finished their execution\n");
	
        printf("Thread final incremented value is: %d\n", thread_cnt);
        
        printf("Execution time is: %f ms\n", exec_time * 1000);
}        
