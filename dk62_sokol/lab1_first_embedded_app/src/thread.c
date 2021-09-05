#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

long long cnt = 0;
long long result = 100;

void *thread_function(void *arg);

int main(void)
{
	int numb_cor_result = 0;
	FILE *pFile;
	pFile = fopen("result.txt", "w");
	pthread_t a_thread, b_thread;
	for(int k = 1; k <= 800; k++){
		result *= 1.01;
		printf("Test number = %i:\n", k);
		numb_cor_result = 0;
		for(int j = 0; j < 10;j++){
			cnt = 0;

			pthread_create(&a_thread, NULL, thread_function, NULL);
			pthread_create(&b_thread, NULL, thread_function, NULL);

			pthread_join(a_thread, NULL);
			pthread_join(b_thread, NULL);

			if(cnt == result * 2){
				numb_cor_result++;
			}
		}
		fprintf(pFile, "%lli %i\n", result, numb_cor_result);
		printf("correct result = %lli\n", result * 2);
		printf("last result = %lli\n", cnt);
		printf("Number of correct results: %i / 10\n", numb_cor_result);
	}
	fclose(pFile);
	return 0;
}
void *thread_function(void *arg)
{
	for (int i; i < result; i++){
		cnt++;
		//usleep(1);
	}
}

