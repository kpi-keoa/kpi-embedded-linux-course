#include <pthread.h>
#include <stdio.h>
#include <time.h>

int count;
int atoi(const char *nptr);
void *potok(void *param);

int iter = 0;

int main(int argc, char *argv[])
{
     clock_t c_s, c_e;
     c_s = clock();

     pthread_t tid0;
     pthread_t tid1;

     pthread_create(&tid0, NULL, potok, NULL);
     pthread_create(&tid1, NULL, potok, NULL);
     pthread_join(tid0, NULL);
     pthread_join(tid1, NULL);

     c_e = clock() - c_s;     
     printf("count = %d\n,    time = %i\n", iter, c_e);
}


void *potok(void *param) 
{
     for(int i; i < 10000000; i++){
          iter++;
     }

     pthread_exit(0);
}
