//Some parts taken from Vadim Kharchuk code
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

volatile unsigned long global = 0;

struct params
{
	long N, K;
};

void *increment_func(void *args)
{
	struct params *steps = args;
	
	for (int i = 0; i < steps->N; i++) {
		global += steps->K;
	}
}

bool is_valid(char *arg, long *result)
{
	long res;
	char sym;
	if (1 != sscanf(arg, "%ld\"%c\"", &res, &sym))
		return true;
	else {
		*result = res;
		return false;
	}
}

int main(int argc, char *argv[]) 
{
	if (argc != 3) {
		fprintf(stderr, "ERR: Function can only be used 2 input arguments!!!\n");
		exit(EXIT_FAILURE);
	}
	
	struct params steps;

   	if (is_valid(argv[1], &steps.K) || is_valid(argv[2], &steps.N)) {
        	fprintf(stderr, "ERR: Wrong input type!!!\n");
        	exit(EXIT_FAILURE);
	}

	pthread_t threads[2];

	pthread_create(&threads[0], NULL, &increment_func, &steps);
	pthread_create(&threads[1], NULL, &increment_func, &steps);

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);
	
	long thread_res = global;
	
	global = 0;
	
	for(long i = 0; i < 2; i++) {
		increment_func(&steps);
	}
	printf("Expected: %ld, got: %ld\n", global, thread_res);
	
	return 0;
}
