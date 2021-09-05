#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

volatile long global_counter = 0;

static const char *const USAGE = (
    "Usage:\n"
    "%s inkerment number"
);

struct Params
{
  long k, n;
};

static bool str_to_int(const char *str, long *resptr)
{
    typeof(errno) _errno = errno;
    errno = 0;

    char *endptr = NULL;
    long conv = strtol(str, &endptr, 10);
    bool retgood = (ERANGE != errno) && finite(conv) && ((str + strlen(str)) == endptr);
    if (retgood)
        *resptr = conv;    // write result only on successful conversion

    errno = _errno;    // restore (probably thread-unsafe)
    return retgood;
}

void *increment(void *args)
{
    struct Params *arr = args;


    for(long i = 0; i < arr->n; i++)
    {
        global_counter += arr->k;
    }
return NULL;
}


int main(int argc, char const *argv[])
{

    if (argc != 3) {
        fprintf(stderr, USAGE, argv[0]);
        exit(EXIT_FAILURE);
    }

    struct Params args;

    if (!str_to_int(argv[1], &args.k) || !str_to_int(argv[2], &args.n))
    {
        fprintf(stderr, "Could not convert input arguments\n");
        exit(EXIT_FAILURE);
    }

    pthread_t thread_1, thread_2;
    pthread_create(&thread_1, NULL, &increment, &args);
    pthread_create(&thread_2, NULL, &increment, &args);

    pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);



    printf("Count with 2 active threads: %li \n", global_counter);
    printf("Expacted count: %li \n", 2*args.k*args.n);
    return 0;
}
