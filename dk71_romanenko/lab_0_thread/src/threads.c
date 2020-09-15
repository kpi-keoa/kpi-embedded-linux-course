//some concepts may be stolen from Vadym Kharchuk
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

int const NUM_OF_THREADS = 2;

volatile long global_var = 0; 
void *counting_thread(void *args)
{
	int steps = *((int *)args); 
	
	for(int i = 0; i < steps; i++) {
		global_var++;	
	} 
}

int main(int argc, char *argv[])
{
	if(argc != NUM_OF_THREADS) {
		perror ("WTF!WHERE THIS THEREADS\n");		
	    	exit (EXIT_FAILURE);
	}
	
	long steps;
	assert (1 == sscanf(argv[1], "%ld", &steps));
	
	pthread_t *threads = calloc(NUM_OF_THREADS, sizeof(*threads));
	
	for (long i = 0; i < NUM_OF_THREADS; i++){
		assert (!pthread_create(&threads[i], NULL, &counting_thread, &steps));
	}
	
	for (long i = 0; i < NUM_OF_THREADS; i++){
		pthread_join(threads[i], NULL);
	}
	
	long later_thread = global_var;
	
	global_var = 0;			
	
	for(long i = 0; i < NUM_OF_THREADS; i++) {
		counting_thread(&steps);
	}
	
	printf("Expected: %ld, got: %ld, it is - %s\n", global_var, later_thread, (global_var == later_thread ? "OK" : "FAIL"));
	
	return 0;

}	
	