// This example is not as clean and concise as the previous one
// Nevertheless, I recommend studying and playing around with it
// To build:
// gcc -std=gnu18 -O2 -Wall -Wextra -Wpedantic ./integrate.c -lm -o ./integrate
//
// Try to change rectangles to trapezoids or parabolas
// Also try to integrate cos(log(x) / x) / x from 0 to 1
// there's special point at 0, so chose some really small starting point near 0
// The right answer is 	0.3233674316...

#define _GNU_SOURCE
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Change this type to float, double, long double and see how the consumed time
// changes and don't forget to change sin to sinf or sinl
typedef double farg_t;

static const char *const USAGE = (
    "Usage:\n"
    "  %s from to nsteps\n"
);


static bool conv_ldbl(const char *str, long double *resptr)
{
    /* 1. Arguments processing should not be fast, it must be correct
     * 2. sscanf and friends *only* extract numbers from string, while letting
     *    for arbitrary junk there. So check that string "is a number",
     *    not only contains it
     * 3. One of the alternatives is to use regexps
     * 4. Floating-points can also be infinite or NaN (not-a-number).
     *    Check it as well
     * 5. Here we rely on short-hand evaluation
     * 6. Save & restore errno. Probably thread-unsafe, but self-contained
     */

    // save errno and zero it before conversion
    typeof(errno) _errno = errno;
    errno = 0;

    char *endptr = NULL;
    long double conv = strtold(str, &endptr);
    bool retgood = (ERANGE != errno) && finite(conv) && ((str + strlen(str)) == endptr);
    if (retgood)
        *resptr = conv;    // write result only on successful conversion

    errno = _errno;    // restore (probably thread-unsafe)
    return retgood;
}


static long long diff_us(struct timespec t1, struct timespec t2)
{
    return (long long)(t2.tv_sec - t1.tv_sec) * 1000000LL + (t2.tv_nsec - t1.tv_nsec) / 1000;
}


farg_t integrate(farg_t start, farg_t stop, unsigned long long nsteps, farg_t (*func)(farg_t))
{
    farg_t res = .0L;
    // ensure our computational math is corrent
    if (start == stop || !nsteps)
        return res;
    farg_t step = (stop - start) / nsteps;
    // we can not integrate if our step is small as 0 -- endless loop
    if (!step)
        return SNANL;    // not a number

    // left to right
    for (farg_t cur = start; cur < stop; cur += step)
        res += step * func(cur);

    return res;
}


int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, USAGE, argv[0]);
        exit(EXIT_FAILURE);
    }

    long double ld_start, ld_stop, ld_steps;
    if (!conv_ldbl(argv[1], &ld_start) || !conv_ldbl(argv[2], &ld_stop)
        || !conv_ldbl(argv[3], &ld_steps)) {
        fprintf(stderr, "Could not convert input arguments\n");
        exit(EXIT_FAILURE);
    }
    unsigned long long nsteps = ld_steps;    // truncate

    struct timespec tstart, tstop;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tstart);

    // rely on implicit downcasting here
    farg_t res = integrate(ld_start, ld_stop, nsteps, &badfunc);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tstop);

    printf("Integral of sin(x) at [%Lf; %Lf] over %Lg points is: %.16Le\n", 
           ld_start, ld_stop, (long double)nsteps, (long double)res);
    printf("Took: %Lf s\n", (long double)diff_us(tstart, tstop) / 1e6);
    return 0;
}
