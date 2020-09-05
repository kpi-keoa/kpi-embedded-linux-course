#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//long global = 0;
volatile long global = 0;

void *thread_func(void *args)
{
	long steps = *((long *)args);
	
	for(int i = 0; i < steps; i++) {
		global++;
	}
}

int main(int argc, char *argv[]) 
{
	if(argc != 2) {
		fprintf(stderr, "It should be one arg\n");
		exit(EXIT_FAILURE);
	}
	
	long steps;
	assert(1 == sscanf(argv[1], "%ld", &steps));
	const int nthreads = 2;
	
	pthread_t *threads = calloc(nthreads,sizeof(*threads));
	
	for(long i = 0; i < nthreads; i++) {
		assert(!pthread_create(&threads[i], NULL, &thread_func, &steps));
	}
	
	for(long i = 0; i < nthreads; i++) {
		pthread_join(threads[i], NULL);
	}
	
	long after_thread = global;
	
	global = 0;		//to test normal result
	
	for(long i = 0; i < nthreads; i++) {
		thread_func(&steps);
	}
	printf("Expected: %ld, got: %ld, it is - %s\n", global, after_thread, (global == after_thread ? "OK" : "FAIL"));
	
	return 0;
}
