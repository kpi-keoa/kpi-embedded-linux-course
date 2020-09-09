#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>

#define SUCCESS 0

uint32_t GlobalVar = 0;

struct stSomeArgs{
	uint8_t step;
	uint32_t amount;
};


/**
 * Function which just increase <GlobalVar> by <step> <amount> times
 * @param args Structure which contain all arguments for foo(step and amount)
 */
void* foo(void * args)
{
	struct stSomeArgs * tmp = (struct stSomeArgs*) args;

	for(uint32_t i = 0; i < tmp->amount; i++)
		GlobalVar += tmp->step;

	return 0;
}

enum enumStatus{
	OK = 0,
	COUDNT_CREATE_THREAT_1,
	COUDNT_CREATE_THREAT_2,
	COUDNT_JOIN_THREAT_1,
	COUDNT_JOIN_THREAT_2,
	WRONG_ARGUMENTS
};


int main(int argc, const char* argv[])
{
	struct stSomeArgs args;

	enum enumStatus ExitStatus = OK;

	// we take arguments from comand line's arguments
	if(argc == 3) {
		args.step = atoi(argv[1]);
		args.amount = atoi(argv[2]);
	} 

	if(!(args.step && args.amount) || argc != 3) {
		printf("Usage:\n\t./thread <step> <amount>\n");
		ExitStatus = WRONG_ARGUMENTS;
		goto fail;
	}

	int status_addr;
	int status;

	pthread_t thread_1;
	pthread_t thread_2;

	status = pthread_create(&thread_1, NULL, foo, (void*) &args);

	if(status != 0) {
		ExitStatus = COUDNT_CREATE_THREAT_1;
		goto fail;
	}


	status = pthread_create(&thread_2, NULL, foo, (void*) &args);

	if(status != 0) {
		ExitStatus = COUDNT_CREATE_THREAT_2;
		goto fail;
	}


	status = pthread_join(thread_1, (void**)&status_addr);

	if(status != 0) {
		ExitStatus = COUDNT_JOIN_THREAT_1;
		goto fail;		
	}


	status = pthread_join(thread_2, (void**)&status_addr);

	if(status != 0) {
		ExitStatus = COUDNT_JOIN_THREAT_2;
		goto fail;		
	}


	printf("Expected GlobalVar is %d\n", args.step * args.amount * 2);
	printf("Real GlobalVar is %d\n", GlobalVar);


	return 0;

fail:
	exit(ExitStatus);
}
