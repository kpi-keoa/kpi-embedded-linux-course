/********************************* 80 *****************************************/
/********************************* 99 ***********************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int x = 0;

void *inc(void *args)
{
	int cycle_times = *(int *)args;
	
	while (++x < cycle_times) {
		usleep(1);
	}

	printf("x increment finished in second thread\n");

	return NULL;
}

int main(int argc, char **argv)
{
	if (argc != 2) {	
		printf("USAGE: %s [cycle_times]\n", argv[0]);
		return -1;
	}	
	int cycle_times = atoi(argv[1]);
	
	printf("Cycle times: %d\n", cycle_times);

	pthread_t inc_thread;
	pthread_create(&inc_thread, NULL, &inc, &cycle_times);


	while (++x < cycle_times) {
		usleep(1);
	}

	printf("x increment finished in main thread\n");

	pthread_join(inc_thread, NULL);

	printf("Result x: %d\n", x);

	return 0;
}
