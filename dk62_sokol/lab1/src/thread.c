#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

void *thread_function();

int main()
{
	pthread_t a_thread;
	int res = pthread_create(&a_thread, NULL, thread_function, NULL);

	clock_t a1 = clock();

	for (int cnt1 = 0; cnt1 < 1000000000; cnt1++);

	clock_t b1 = clock();
	printf("cnt1 = %f second\n", (float)(b1 - a1)/CLOCKS_PER_SEC);	

	res = pthread_join(a_thread, NULL);
	return 0;
}

void *thread_function()
{
	clock_t a2 = clock();

	for (int cnt2 = 0; cnt2 < 1000000000; cnt2++);

	clock_t  b2 = clock();
	printf("cnt2 = %f second\n", (float)(b2 - a2)/CLOCKS_PER_SEC);


}

