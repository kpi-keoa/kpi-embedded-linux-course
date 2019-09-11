#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

const long long NUM = 2;
const long long count_to = 1000000L;
 
long long global_in = 0;

void *count_thread(void *arg)
{

		long long to = *(long long *)arg;
 	
		for(long long i = 0; i < to; i++){
		        global_in++; 
		}
		
		printf("Current value - %d\n", global_in);
		return 0;
}
 
int main(void)
{

	pthread_t thread[NUM];

	for(long long i = 0; i < NUM; i++) {
		int status = pthread_create(&thread[i], NULL, &count_thread, (void *)&count_to);
		if(status) {
			printf("Error: Thread is not created: err%d\n", status);
			exit(EXIT_FAILURE);
		} else {
 			printf("Thread was created\n");
        	}
 	}
 	
	for(int i = 0; i < NUM; i++) {
		int status = pthread_join(thread[i], NULL);
		if(status) {
			printf("Error: Cant join thread: err%d\n", status);
			exit(EXIT_FAILURE);
		} else {
 			printf("Thread was joined\n");
		
        	}
 	}
	
 	printf("Result after running two threads: %lld", global_in);
 	
 	return 0;
}
 