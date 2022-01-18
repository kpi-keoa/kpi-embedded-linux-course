#include "stdio.h"
#include "stdlib.h"
#include "argp.h"

enum Errors {
    EXECUTION_SUCCESSFUL,
    NOT_ENOUGH_ARGUMENT,
    TOO_MANY_ARGUMENTS,
};

enum VarType {
	BIN,
	DEC,
	HEX
};

enum Operation {
	SET,
	UNSET,
	FLIP
};

int setbit(const int value, const int position);
int unsetbit(const int value, const int position);
int switchbit(const int value, const int position);
