#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// We use macrodefs from math, but linking
// with -lm is not strictly required
#include <math.h>

static const char *const USAGE = ("Usage:\n"
                                  "  %s [NUMBER]...\n");

// We need to use it at least twice. So keep it DRY
/**
 * conv_dbl() - Convert string to double checking for errors.
 * @str:    a character string
 * @resptr: conversion result
 *
 * Return: true on successful conversion, false otherwise
 */
static bool conv_dbl(const char *str, double *resptr)
{
    /* 1. Arguments processing should not be fast, it must be correct
     * 2. sscanf and friends *only* extract numbers from string, while letting for
     *    arbitrary junk there. So check that string "is a number", not only contains it
     * 3. One of the alternatives is to use regexps
     * 4. Floating-points can also be infinite or NaN (not-a-number). Check it too
     * 5. Here we rely on short-hand evaluation
     * 6. Save & restore errno. Probably thread-unsafe, but self-contained
     */

    // save errno and zero it before conversion
    typeof(errno) _errno = errno;
    errno = 0;

    char *endptr = NULL;
    double conv = strtod(str, &endptr);
    bool retgood = (ERANGE != errno) && finite(conv) && ((str + strlen(str)) == endptr);
    if (retgood)
        *resptr = conv;    // write result only on successful conversion

    errno = _errno;    // restore (probably thread-unsafe)
    return retgood;
}


int main(int argc, char *argv[])
{
    if (argc <= 1) {
        fprintf(stderr, USAGE, argv[0]);
        goto fail;
    }

    double res;
    int cnt = 1;
    // we need to read first one before iterating
    // assigning res = -infinity will work as well
    if (!conv_dbl(argv[cnt], &res))
        goto argerr;

    for (++cnt; cnt < argc; cnt++) {
        double val;
        if (!conv_dbl(argv[cnt], &val))
            goto argerr;

        if (val > res)
            res = val;
    }

    printf("%g\n", res);
    return 0;
argerr:
    fprintf(stderr, "Wrong argument #%d \"%s\"\n", cnt, argv[cnt]);
fail:
    exit(EXIT_FAILURE);
}
