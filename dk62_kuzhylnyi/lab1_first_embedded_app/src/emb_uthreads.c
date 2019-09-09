#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

inline void io_clear(void)
{
    printf("\033c");
}

struct counter_set {
	volatile uint32_t opt_shared_counter;
	uint32_t nopt_shared_counter;
	volatile uint32_t pivot_value;
};

void *pthread_func(void *user_data);

int main(void)
{
	static const unsigned threads_num = 2;
	pthread_t threads_bunch[threads_num];

	int retval;

	/* Change 'base_value' in range [0..2^32-1] to provide an arch-"independency" */
	static const uint32_t base_value = 1000000;

	struct counter_set test_cnt_set = {.opt_shared_counter = 0,
						.nopt_shared_counter = 0,
						.pivot_value = base_value
						};

	/* Run independent threads each of which will execute function */
	for(unsigned the_thread=0; the_thread < threads_num; the_thread++) {
		retval = pthread_create(&threads_bunch[the_thread], NULL, pthread_func, (void*)&test_cnt_set);
		if(0 != retval) {
			fprintf(stdout, "Error - pthread_create() return code: %d\n", retval);
			exit(EXIT_FAILURE);
		}
		pthread_join(threads_bunch[the_thread], NULL);
	}
	
	usleep(100);

	fprintf(stdout, "Opt variable: %d\t\n", test_cnt_set.opt_shared_counter);
	fprintf(stdout, "Non-opt variable: %d\t\n", test_cnt_set.nopt_shared_counter);
	fprintf(stdout, "Pivot: %d\t\n", test_cnt_set.pivot_value);

	fprintf(stdout, "Expected result: %d\t\n", test_cnt_set.pivot_value * threads_num); 

	exit(EXIT_SUCCESS);
}

void *pthread_func(void *user_data)
{
	struct counter_set *cntset_thread = (struct counter_set *)user_data;

	/* increment both variables simultaneously in separated threads*/
	for(unsigned long long pthread_cnt=0; pthread_cnt < cntset_thread->pivot_value; pthread_cnt++) {
		fprintf(stdout, "Opt variable: %d\t\n", ++cntset_thread->opt_shared_counter);
		fprintf(stdout, "Non-opt variable: %d\t\n", ++cntset_thread->nopt_shared_counter);
	 	io_clear();
		usleep(100);
	}
	return NULL;
}


