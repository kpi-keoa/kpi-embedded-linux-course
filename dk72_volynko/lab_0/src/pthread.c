#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int global_var = 0;

struct parameters
{
	int n , k;
};

static void* add_func(void* args)
{
	struct parameters *par =  args;

	for(int i = 0; i < par->n; i++) {
		global_var += par->k;
	}
	return 0;
}


int main(int argc, char *argv[])
{
	if(argc != 3) {
		fprintf(stderr, "Please, enter the right number of argument\n");
		exit(EXIT_FAILURE);
	}	

	int N = strtoul(argv[1], NULL, 10);
	int K = strtoul(argv[2], NULL, 10);

	struct parameters par = { .n = N, .k = K};

	pthread_t pthread [2];

	for(int i = 0; i < 2; i++) 
		pthread_create(&pthread[i], NULL, &add_func, &par);
		
	for(int i = 0; i < 2; i++) 
		pthread_join(pthread[i], NULL);
	

	printf("Expected result: %d \n", 2 * N * K);
	printf("The real result: %d \n", global_var);

	return 0;
}


