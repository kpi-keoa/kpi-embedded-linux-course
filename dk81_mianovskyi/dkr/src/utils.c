#include "utils.h"

const char *parse_err[] = {
	"",
	"Too few arguments!\n",
	"Too many arguments!\n",
	"Not a number!\n",
	"Number is out of range!\n",
	"Number is negative!\n",
	"Verbosity option is turned on\n",
	"New game was started\n",
	"Checking argument\n",
	"Argument has passed successfuly!\n"
};

const char *color_code[] = {
	"\e[31;1m",
	"\e[34;1m",
	"\e[32;1m",
	"\e[33;1m",
	"\e[37;1m"
};

static void parse_check(struct arguments *args)
{
	fprintf(args->parse_stat >= 6 ? stdout : stderr, "%s%s%s",
		args->parse_stat >= 6 ? color(BLUE) : color(RED),
		args->verbose && args->parse_stat != 0 ? (args->parse_stat >= 6 ? "Parser: " : "Error: ") : "",
		(args->parse_stat < 6 || args->verbose) ? parse_err[args->parse_stat] : "");
	if(args->parse_stat < 6 && args->parse_stat > 0)
		exit(args->parse_stat);
	return;
}

const char * const color(enum colors col)
{
	return color_code[col];
}

int parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *args = state->input;
	int i = 0;
	int len;
	args->parse_stat = NO_ERRORS;
	switch(key)
	{
	case 'v':
	args->verbose = 1;
	args->parse_stat = VERB_IS_ON;
	parse_check(args);
	break;
	case 'n':
	args->new_game = 1;
	args->parse_stat = NEW_GAME_START;
	parse_check(args);
	break;
	case ARGP_KEY_ARG:
	if(NULL == arg) goto skip;
	
	if(state->arg_num + 1 > 1) {
		args->parse_stat = TOO_MANY_ARGS;
		goto skip;
	}

	args->parse_stat = CHECK_ARGS;
	parse_check(args);

	len = strlen(arg);
	while((i < len) && (arg[i] >= 48) && (arg[i] <= 57)) i++;

	if(((i == len) && (i > 1)) || (strcmp(arg, "6")>0) || !strcmp(arg, "0")) {
		args->parse_stat = OUT_OF_RANGE;
		goto skip;
	} else if (i < len) {
		args->parse_stat = NEGATIVE_NUM;
		
		if(arg[i] != '-') args->parse_stat = NOT_A_NUM;

		goto skip;
	}
	args->parse_stat = CHECK_OK;
	
	args->num = atoi(arg);
skip:
	parse_check(args);
	break;
	case ARGP_KEY_END:
	if(state->arg_num == 0) args->parse_stat = TOO_FEW_ARGS;
	parse_check(args);	
	break;
	}	
	args->parse_stat = NO_ERRORS;
	return 0;
}
