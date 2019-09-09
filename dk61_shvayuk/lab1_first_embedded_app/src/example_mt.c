#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int cnt = 0;

/* This function runs in each thread */
void *threadfunc(void *args)
{	
	while (cnt < 10000) {
		cnt++;
		usleep(1000);
	}
	return NULL;
}


int main(int argc, char *argv[])
{
	const int num_threads = 2;
	
	pthread_t *threads = malloc(num_threads * sizeof(*threads));
	
    if (NULL == threads) {
        printf("Could not allocate dynamic memory, abort.\n");
        return 1;
    }
        
	for (int i = 0; i < num_threads; i++) {
		if (0 != pthread_create(&threads[i], NULL, &threadfunc, NULL)) {
            printf("Could not create the thread, abort.\n");
            return 3;
        }
    }
	
	printf("Waiting for threads...\n:");
	
	if (0 != pthread_join(threads[0], NULL)) {
        printf("Could not join the thread, abort.\n");
        return 4;
    }
    
	if (0 != pthread_join(threads[1], NULL)) {
        printf("Could not join the thread, abort.\n");
        return 5;
    }
	
	printf("Counter value: %i\n", cnt);
	
	return 0;
}
