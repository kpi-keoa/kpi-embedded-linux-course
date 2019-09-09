#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
 
#define ERROR_CREATE_THREAD 1
#define ERROR_JOIN_THREAD   2
#define SUCCESS               0
#define NUM 2

typedef struct {
	int in;
}value;

void* print (void* arg)
{
	value *args = (value*)arg;
	
	int counter = 0;
	
	for(args->in; args->in < 1000000; args->in++){
		//printf("Current value - %d\n", args->in);
	}
	printf("Current value - %d\n", args->in);
	return SUCCESS;
}

int main(int argc, char *argv[])
{
	int status, status_from_thread, integer;
	
	pthread_t thread [NUM];
	
	value arg[NUM];
	
	
	for (int i = 0; i < NUM; i++){
		arg[i].in = 2;
	}
	
	for(int i = 0; i < NUM; i++){
		status = pthread_create(&thread[i], NULL, &print, &arg[i]);
		if(status != SUCCESS){
			printf("Error: Thread is not created\n");
			exit(ERROR_CREATE_THREAD);
		}else 
			printf("Thread was created\n");
	}
	
	for(int i = 0; i < NUM; i++){
		status = pthread_join(thread[i], (void**) &status_from_thread);
		if(status_from_thread != SUCCESS){
			printf("Error: Cant join thread \n");
			exit(ERROR_JOIN_THREAD);
		}else
			printf("Thread was joined\n");
			
	}
		
	return 0;

} 