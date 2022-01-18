#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum Verbose {
    VERBOSE_DISABLE,
    VERBOSE_ENABLE
};

enum TRIM {
    TRIM_DISABLE,
    TRIM_ENABLE
};

enum Errors {
    EXECUTION_SUCCESSFUL,
    NO_ARGUMENTS,
    FILE_FAIL
};