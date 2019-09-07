#include <pthread.h>
#include <stdio.h>


long long int count; 
long long int atoi(const char *nptr);
void *thread_func(void *argument);

int main(int argc, char *argv[]) 
{
	/** 
	 * tid, tid2 - threads identifier  
	 */
	pthread_t tid; 
	pthread_t tid2; 
 		
	if (argc != 2) {
		fprintf(stderr,"Invalid number of arguments\n");
  		return -1;
	}

	if (atoi(argv[1]) < 0) {
  		fprintf(stderr,"Please, try positive value\n");
  		return -1;
	}
	
	/**
	 * We should call set task cpu() on a create threads (2, in our case). 
	 */
	pthread_create(&tid,NULL,thread_func,argv[1]);
	pthread_create(&tid2,NULL,thread_func,argv[1]);

  	pthread_join(tid,NULL);
	pthread_join(tid2,NULL);
  	printf("count = %llu\n",count);
}

/**
 * thread_func - function of increment global counter. 
 * @argument: pointer, which contains address of thread argument.   
 */

void *thread_func(void *argument) 
{
	
	long long int i, upper = atoi(argument);

  	if (upper > 0) {
    		for (i = 1; i <= upper; i++)
    			count++;
  	}

  	pthread_exit(NULL);
}


