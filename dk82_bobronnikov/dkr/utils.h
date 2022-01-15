#include "stdio.h"
#include "stdlib.h"
#include "argp.h"

enum Errors {
    EXECUTION_SUCCESSFUL,
    NO_ARGUMENT,
    TOO_MANY_ARGUMENTS,
    WRONG_ARGUMENT
};

FILE *fp;
__uint8_t generate_num(void);
void read_stats(int *score, int *count);
void write_scores(int score, int count);
