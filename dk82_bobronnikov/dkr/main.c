#include "utils.h"

const char *argp_program_version = "0.1";

static char doc[] =
  "Guessing game - enter the number from 0 to 15\n" 
  "If it`s equal to PC`s - you win";

static char args_doc[] = "NUM - your number from 0 to 15";

static struct argp_option options[] = {
  {"verbose", 'v', 0, 0, "Produce verbose output" },
  {"new", 'n', 0, 0, "Start new game with erasing stats" },
  { 0 }
};

struct arguments
{
  char *args[1];
  int verbose, new;
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
    case 'n':
      arguments->new = 1;
      break;

    case ARGP_KEY_ARG:
      if (state->arg_num >= 1) {
        fprintf(stderr, "Too many arguments!\n");
        exit(TOO_MANY_ARGUMENTS);
        argp_usage (state);
	}
      arguments->args[state->arg_num] = arg;
      break;

    case ARGP_KEY_END:
      if (state->arg_num < 1) {
        fprintf(stderr, "No argument!\n");
        exit(NO_ARGUMENT);
        argp_usage (state);
	}
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

struct current_score
{
	__uint8_t user;
	__uint8_t pc;
};

int main(int argc, char *argv[])
{
	struct arguments arguments;
	struct current_score current_score;
	int score, count;
	arguments.verbose = 0;
	arguments.new = 0;

	/* Parse our arguments; every option seen by parse_opt will
     be reflected in arguments. */
	argp_parse (&argp, argc, argv, 0, 0, &arguments);
	
	// Checking the entered number
	current_score.user = atoi(arguments.args[0]);
	if (current_score.user < 0 || current_score.user > 15) {
		fprintf(stderr, "Wrong number!\n");
        exit(WRONG_ARGUMENT);
	}
	
	// Generating the random number for PC player
	current_score.pc = generate_num();
	
	// Make zero score in case of newgame, else read from file
	if (arguments.new) {
		score = 0, count = 0;
	}
	else read_stats(&score, &count);
	
	// If verbosity is on, print user and pc nums
	if (arguments.verbose) 
		fprintf(stdout, "pc num: %d\nyour num: %d\n", 
			current_score.pc, current_score.user);
			
	// Add 1 to game counter
	// If PC and user nums are equal - add score
	count++;
	if (current_score.pc == current_score.user) score++;
	
	// Store the stats to file
	write_scores(score, count);
	exit(EXECUTION_SUCCESSFUL);
}
