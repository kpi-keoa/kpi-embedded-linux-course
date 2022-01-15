#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum errors {
	NO_ERRORS,
	TOO_FEW_ARGS,
	TOO_MANY_ARGS,
	NOT_A_NUM,
	OUT_OF_RANGE,
	NEGATIVE_NUM,
	VERB_IS_ON,
	NEW_GAME_START,
	CHECK_ARGS,
	CHECK_OK
};

enum colors {
	RED,
	BLUE,
	GREEN,
	YELLOW,
	WHITE
};

struct arguments {
	enum errors parse_stat;
	int verbose;
	int new_game;
	int num;
};

const char * const color(enum colors col);
int parse_opt(int key, char *arg, struct argp_state *state);
