#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum Verbose {
    VERBOSE_DIS,
    VERBOSE_EN
};

enum choose {
	ADD_PERSON,
	REMOVE_PERSON,
	LIST_TABLE
};

enum Errors {
    SUCCESSFUL,
    NO_ARGUMENTS,
    NCORRECT_ARG,
};


