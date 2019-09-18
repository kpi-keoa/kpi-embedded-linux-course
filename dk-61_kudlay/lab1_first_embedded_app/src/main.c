// Created by CYB3RSP1D3R

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>

int global_inc = 0;

// Thread function for global_incrementation
void *global_increment(void *limitcount);

int main(int argc, char **argv)
{
        int limitcount;

        // Checking for a right arguements
        //  One additional digit arguement should be when user calls the application
        if (argc != 2) {
                printf("Error: global_incorrect number of arguements.\n");
                exit(EXIT_FAILURE);
        }
        char *endptr;

        limitcount = strtol(argv[1], &endptr, 10);

        if (*endptr != '\0') {
                printf("Error: arguement could be anything except digit.\n");
                exit(EXIT_FAILURE);
        }

        // Initialising variables for two threads
        pthread_t thread0, thread1;

        clock_t time_begin = clock();

        // Declaring 2 threads for for counting
        pthread_create(&thread0, NULL, &global_increment, &limitcount);
        pthread_create(&thread1, NULL, &global_increment, &limitcount);

        // Wait until 2 threads finish their work
        pthread_join(thread0, NULL);
        pthread_join(thread1, NULL);

        time_t time_end = clock();
        printf("Time spent for counting using 2 cores:\n%li us\nCounted value is:\n%i\n",
                (time_end-time_begin)/(CLOCKS_PER_SEC/1000000), global_inc);


        exit(0);
}

void *global_increment(void *limitcount)
{
        int limit = *((int*)limitcount);

        // global_inc is global_incremented limitcount times in 1 thread
        for (int i = 0; i < limit; i++) {
                global_inc++;
        }

        pthread_exit(0);
}
