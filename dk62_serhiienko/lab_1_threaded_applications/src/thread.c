#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

// TODO: add .gitignore, README.rst, fix Makefile, add license
volatile long long int global_cnt = 0;
struct arg_struct {
        bool mutex_enable;
        pthread_mutex_t mutex;
        long long int inc_amnt; // how many times thread will increment global_cnt
        bool is_verbose;
};
/**
 * inc_foo() - function that increments global_counter by 1 in a loop
 * @arg1: pointer to void.
 * 
 *  Function is passed to the thread, when it's created.
 *  As an argument, the function takes a structure.
 *  Before passing it to the function, the struct is casted to void*.
 *  Then it's casted back to the 'struct arg_struct'.
 * 
 * 
 */
void* inc_foo(void*);
int main (int argc, char** argv) 
{      
        struct arg_struct* foo_args = malloc(sizeof(struct arg_struct));
        int opt;
        while((opt = getopt(argc, argv, "Mhva:")) != -1) {
                switch(opt) {
                case 'M':
                        pthread_mutex_init(&foo_args->mutex, NULL);
                        foo_args->mutex_enable = true;
                        break;
                case 'a':
                        foo_args->inc_amnt = atoi(optarg);
                        break;
                case '?':
                //fall-through
                case 'h':
                        printf("usage : ./a.out -a <value>\n");
                        printf("'-M' for enabling mutex.\n");
                        printf("'-a' <value> setting value to which threads will count to. \n");
                        printf("'-v' to have more output information.\n");
                        printf("'-h' for help.\n");
                        return 0;
                        break;
                        case 'v':
                        foo_args->is_verbose = true;
                        break;

                }
        }

        if(foo_args->inc_amnt <= 0) {
                printf("-a <value> is mandatory! \n");
                return 0;
        }

        pthread_t thr1, thr2;
        pthread_create(&thr1, NULL, inc_foo, (void *)foo_args);
        pthread_create(&thr2, NULL, inc_foo, (void *)foo_args);
        if(foo_args->is_verbose) {
                printf("Threads created !\n");
                if(foo_args->mutex_enable)
                        printf("Running threads blocked by mutex..\n");
                }

        pthread_join(thr1, NULL);
        pthread_join(thr2, NULL);

        free(foo_args);
        printf("global cnt is :%lld\n", global_cnt);
        return 0;
}

void* inc_foo(void* args)
{
        struct arg_struct* arguments = (struct arg_struct *)args;
        if(arguments->mutex_enable) {
                pthread_mutex_lock(&arguments->mutex);
                if(arguments->is_verbose)
                        printf("Thread locked !\n");    
        }
        for(volatile long long int i = 0; i < arguments->inc_amnt; i++)
                global_cnt++;

        if(arguments -> mutex_enable) {
                pthread_mutex_unlock(&arguments->mutex);
                if(arguments->is_verbose)
                        printf("Thread unlocked !\n");
        }
        return NULL;
}
