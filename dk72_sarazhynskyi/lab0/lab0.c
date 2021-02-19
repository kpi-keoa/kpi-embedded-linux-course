#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

//volatile
uint64_t globalvar = 0;

static int conv_u64(const char *str, uint64_t *resptr)
{
    //stolen from lecture examples
    typeof(errno) _errno = errno;
    errno = 0;

    char *endptr;
    uint64_t conv = strtoul(str, &endptr, 10);
    int retgood = (ERANGE != errno) && ((str + strlen(str)) == endptr);
    if (retgood)
        *resptr = conv;

    errno = _errno;
    return retgood;
}

void *foo(void *end)
{
    uint64_t _end = *(uint64_t *)end;
    for (uint64_t i = 0; i < _end; ++i)
        ++globalvar;
    // __asm__ volatile (
    // "lock "
    // "incq %0"::"m"(globalvar));
    return 0;
}

int main(int argc, char const *argv[])
{
    uint64_t nthreads, end;
    pthread_t *threads = NULL;
    int status;

    //read args (cringy but enough for not to crash)
    if (argc != 3) {
        fprintf(stderr, "USAGE: ./thread <nthreads> <end>\n");
        errno = EINVAL;
        goto err;
    } else {
        if ((conv_u64(argv[1], &nthreads) && conv_u64(argv[2], &end)) == 0) {
            errno = EINVAL;
            goto err;
        }
    }

    threads = (pthread_t *)malloc(nthreads * sizeof(pthread_t));
    if (!threads) {
        errno = ENOMEM;
        goto err;
    }

    for (uint64_t i = 0; i < nthreads; ++i) {
        status = pthread_create(&threads[i], NULL, foo, &end);
        if (status) {
            errno = EAGAIN;
            goto err;
        }
    }

    for (uint64_t i = 0; i < nthreads; ++i) {
        status = pthread_join(threads[i], NULL);
        if (status) {
            errno = EAGAIN;
            goto err;
        }
    }

    printf("Expected globalvar is %ld\n", end * nthreads);
    printf("Real globalvar is %ld\n", globalvar);
    return 0;

err:
    if (threads) {
        typeof(errno) _errno = errno;
        free(threads);
        errno = _errno;
    }
    exit(errno);
}
