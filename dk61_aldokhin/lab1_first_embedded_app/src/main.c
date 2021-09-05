#include <pthread.h>
#include <stdio.h>
#include <time.h>

int count;
void *thread_iter(void *param);

int iter = 0; // constant used for iteration and testing

int main(void)
{
        clock_t c_s, c_e; // variables for calculating program execution speed
        c_s = clock();

        pthread_t tid0; // stream creation
        pthread_t tid1;

        pthread_create(&tid0, NULL, &thread_iter, NULL);
        pthread_create(&tid1, NULL, &thread_iter, NULL);
        pthread_join(tid0, NULL); // waiting for thread to finish
        pthread_join(tid1, NULL);

        c_e = clock() - c_s;
        printf("count = %d\ntime = %i\n", iter, c_e);
}

/* Number iteration function.
 * Used to call in a stream
 */
void *thread_iter(void *param) 
{
        for (int i; i < 10000000; i++) { // constant - given in the task, used for comparison
                iter++;
        }

        pthread_exit(0);
}
