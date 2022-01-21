#include "utils.h"

const char *argp_program_version = "0.1";

static char doc[] =
  "Bit calculator that can perform operations SET, UNSET, FLIP\n" 
  "Can take the decimal, binary and hex numbers";

static char args_doc[] = "NUM - number in DEC, BIN or HEX\n"
	"POSITION - bit position to change strting with 0"
	"OPERATION - options SET, UNSET, FLIP(change)";

static struct argp_option options[] = {
  {"verbose", 'v', 0, 0, "Produce verbose output" },
  {"set", 's', 0, 0, "Set selected bit to 1" },
  {"unset", 'u', 0, 0, "Set selected bit to 0" },
  {"flip", 'f', 0, 0, "Reverse selected bit" },
  { 0 }
};

struct arguments
{
  char *args[2];
  int verbose, set, unset, flip;
};

// Parse a single option
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'v':
      arguments->verbose = 1;
      break;
    case 's':
      arguments->set = 1;
      break;
    case 'u':
      arguments->unset = 1;
      break;
    case 'f':
      arguments->flip = 1;
      break;

    case ARGP_KEY_ARG:
      if (state->arg_num >= 2) {
        fprintf(stderr, "Too many arguments!\n");
        exit(TOO_MANY_ARGUMENTS);
        argp_usage (state);
	}
      arguments->args[state->arg_num] = arg;
      break;

    case ARGP_KEY_END:
      if (state->arg_num < 2) {
        fprintf(stderr, "Not enough arguments!\n");
        exit(NOT_ENOUGH_ARGUMENT);
        argp_usage (state);
	}
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char *argv[])
{
	struct arguments arguments;
	
	int vartype = DEC;
	int operation;
	int dec = 0;
	
	arguments.verbose = 0;
	arguments.set = 0;
	arguments.unset = 0;
	arguments.flip = 0;

	/* Parse our arguments; every option seen by parse_opt will
     be reflected in arguments. */
	argp_parse (&argp, argc, argv, 0, 0, &arguments);
	
	char *str = arguments.args[0];
	int position = atoi(arguments.args[1]);
	
	if(arguments.verbose) printf("Verbosity is on!\n");
	if(arguments.set) operation = SET;
	if(arguments.unset) operation = UNSET;
	if(arguments.flip) operation = FLIP;
	if(arguments.verbose) printf("string: %s\n", str);
	
	//if(strcmp(&str[1], "x") == 0) vartype = HEX;
	if(str[1] == 'x') vartype = HEX;
	if(str[1] == 'b') vartype = BIN;
	
	switch(vartype)
	{
		case BIN:
			{
				dec = (int)strtol(str, NULL, 2);
				break;
			}
		case DEC:
			{
				dec = (int)strtol(str, NULL, 10);
				break;
			}
		case HEX:
			{
				dec = (int)strtol(str, NULL, 16);
				break;
			}
	}
	
	if(arguments.verbose) printf("start: %d\n", dec);
	
	switch(operation)
	{
		case SET:
			{
				if(arguments.verbose) printf("setbit: ");
				printf("%d\n", setbit(dec,position));
				break;
			}
		case UNSET:
			{
				if(arguments.verbose) printf("unsetbit: ");
				printf("%d\n", unsetbit(dec,position));
				break;
			}
		case FLIP:
			{
				if(arguments.verbose) printf("switchbit: ");
				printf("%d\n", switchbit(dec,position));
				break;
			}
	}
	
	exit(EXECUTION_SUCCESSFUL);
}
