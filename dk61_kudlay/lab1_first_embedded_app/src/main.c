#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

int global_inc = 0;

void *global_increment(void *limit);

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Error: wrong number of arguements!\n");
		exit(EXIT_FAILURE);
	}
	
	char *endp;
	int limitcount = strtol(argv[1], &endp, 10);
	
	if (*endp != '\0') {
		printf("Error: wrong arguement format!\n");
		exit(EXIT_FAILURE);
	}
	
	pthread_t thread0, thread1;
	
	clock_t time_begin = clock();
	
	pthread_create(&thread0, NULL, &global_increment, &limitcount);
	pthread_create(&thread1, NULL, &global_increment, &limitcount);
	
	pthread_join(thread0, NULL);
	pthread_join(thread1, NULL);
	
	clock_t time_end = clock();
	
	printf("Counted value using 2 threads: %i\nTime spent: %li us\n",
		 global_inc, (time_end - time_begin)/(CLOCKS_PER_SEC/1000000));
	
	exit(EXIT_SUCCESS);
}

void *global_increment(void *limit) 
{
	int limitcount = *((int*) limit);
	
	for (int i = 0; i < limitcount; i++) {
		global_inc++;
	}
	
	pthread_exit(0);
}
