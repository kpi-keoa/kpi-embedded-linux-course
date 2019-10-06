#include <pthread.h>
#include <stdio.h>

unsigned int global_cnt = 0;

void *first_thread(void *param);


int main(void)
{
	pthread_t first_ident;
	pthread_t second_ident;
	pthread_create(&first_ident, NULL, &first_thread, NULL);
	pthread_create(&second_ident, NULL, &first_thread, NULL);
	
	pthread_join(first_ident, NULL);
	pthread_join(second_ident, NULL);

	printf("Global counter count to %i\n", global_cnt);
}

void *first_thread(void *param)
{
	for (int i = 0; i < 100000000; i++) {
	
		global_cnt ++;	
	}
}
