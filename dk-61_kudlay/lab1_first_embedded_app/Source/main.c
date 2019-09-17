//Created by CYB3RSP1D3R

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>

int inc = 0;

//Thread function for incrementation
void *increment(void *limitcount);

int main (int argc, char **argv)
{
        int limitcount;

        //Checking for a right arguements
        // One additional digit arguement should be when user calls the application
        if(argc != 2){
                printf("Error: Incorrect number of arguements\n");
                exit(1);
        }
        if(!isdigit(argv[1][0])){
                printf("Error: arguement is not digit\n");
                exit(1);
        }

        limitcount = strtol(argv[1], NULL, 10);

        //Initialising variables for two threads
        pthread_t thread0, thread1;
        pthread_attr_t attr_thr0, attr_thr1;


        //Initialialising of threads attributes
        pthread_attr_init(&attr_thr0);
        pthread_attr_init(&attr_thr1);

        clock_t time_begin = clock();

        //Declaring 2 threads for for counting
        pthread_create(&thread0, &attr_thr0, increment, &limitcount);
        pthread_create(&thread1, &attr_thr1, increment, &limitcount);

        //Wait until 2 threads finish their work
        pthread_join(thread0,NULL);
        pthread_join(thread1,NULL);

        time_t time_end = clock();
        printf("Time spent for counting using 2 cores:\n%li us\n", (time_end-time_begin)/(CLOCKS_PER_SEC/1000000));

        exit(0);
}

void *increment(void *limitcount)
{
        int limit = *((int*)limitcount);

        //inc is incrementing until its value is less then limit
        //Incrementation has to be in a body of a cycle due to a parallel counting
        while(inc < limit){
                inc++;
        }

        pthread_exit(0);
}
