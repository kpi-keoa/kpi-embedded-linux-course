#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#define IO_CLEAR() printf("\033c")

struct counter_set{
	volatile uint32_t opt_shared_counter;
	uint32_t nopt_shared_counter;
	volatile uint32_t pivot_value;
};


void *pthread_func(void *user_data);

int main()
{
	pthread_t opt_thread, 
		nopt_thread;

	int retval;

	/* Change 'base_value' in range [0..2^32-1] to provide a full multithread tests */
	static uint32_t base_value = 1000000;
	struct counter_set test_cnt_set = {.opt_shared_counter = 0,
						.nopt_shared_counter = 0,
						
						.pivot_value = base_value
						};

	/* Create independent threads each of which will execute function */
	retval = pthread_create(&opt_thread, NULL, pthread_func, (void*)&test_cnt_set);
	if(retval){
		fprintf(stdout,"Error - pthread_create() return code: %d\n",retval);
		exit(EXIT_FAILURE);
	}

	retval = pthread_create(&nopt_thread, NULL, pthread_func, (void*)&test_cnt_set);
	if(retval){
		fprintf(stderr,"Error - pthread_create() return code: %d\n",retval);
		exit(EXIT_FAILURE);
	}

	/* Wait till threads are complete before main continues */

	pthread_join(opt_thread, NULL);
	pthread_join(nopt_thread, NULL); 
	
	usleep(100);

	fprintf(stdout, "Opt variable: %d\t\n", test_cnt_set.opt_shared_counter);
	fprintf(stdout, "Non-opt variable: %d\t\n", test_cnt_set.nopt_shared_counter);
	fprintf(stdout, "Pivot: %d\t\n", test_cnt_set.pivot_value);

	/* The output of 2 threads = pivot value * 2 */
	fprintf(stdout, "Expected result: %d\t\n", test_cnt_set.pivot_value * 2); 
	exit(EXIT_SUCCESS);
}

void *pthread_func(void *user_data)
{
	struct counter_set *cntset_thread; 
	cntset_thread = (struct counter_set *)user_data;
	
	/* increment both variables simultaneously in separated threads*/
	for(register long int i=0;i<cntset_thread->pivot_value;i++){
		fprintf(stdout, "Opt variable: %d\t\n", ++cntset_thread->opt_shared_counter);
		fprintf(stdout, "Non-opt variable: %d\t\n", ++cntset_thread->nopt_shared_counter);
	 	IO_CLEAR();
		usleep(300);
	}
}


