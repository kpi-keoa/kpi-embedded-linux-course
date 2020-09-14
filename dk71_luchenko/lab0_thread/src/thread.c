//Some parts taken from Vadim Kharchuk code
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
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

int check(char *arg, long *result)
{
	int res;
	char sym;
	if (1 != sscanf(arg, "%d\"%c\"", &res, &sym))
		return 1;
	else *result = res;
		return 0;
}

int main(int argc, char *argv[]) 
{
	if (argc != 3) {
		fprintf(stderr, "\nERR: Function can only be used 2 input arguments!!!\n\n");
		exit(EXIT_FAILURE);
	}
	
	struct params steps;

   	if (check(argv[1], &steps.K) || check(argv[2], &steps.N)) {
        	fprintf(stderr, "\nERR: Wrong input type!!!\n\n");
        	exit(EXIT_FAILURE);
	}

	pthread_t threads_1, threads_2;	

	pthread_create(&threads_1, NULL, &increment_func, &steps);
	pthread_create(&threads_2, NULL, &increment_func, &steps);

	pthread_join(threads_1, NULL);
	pthread_join(threads_2, NULL);
	
	long thread_res = global;
	
	global = 0;
	
	for(long i = 0; i < 2; i++) {
		increment_func(&steps);
	}
	printf("\n\nExpected: %ld, got: %ld\n\n\n", global, thread_res);
	
	return 0;
}
