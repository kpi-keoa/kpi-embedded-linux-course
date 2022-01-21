#include "my_rand.h"

const char *argp_program_version = "version 0.1";

const char argp_msg[] = {
	"Whelcome to the game!\n"
	"The main purpouse of this game is to guess a number,\n"
	"that is set by computer. Computer sets only positive\n"
	"numbers from 1 to 6, so should put only numbers, that\n"
	"fit in this diapazone. Good luck!\n"
};

struct argp_option options[] =
	{
		{0, 0, 0, 0, "Game options:", 1},
		{"new", 'n', 0, 0, "Start a new game"},
		{0, 0, 0, 0, "Informational options:", 2},
		{"verbose", 'v', 0, 0, "Give extra info"},
		{0}
	};


struct argp my_argp = { options, parse_opt, "Guess the number game", argp_msg};


int main(int argc, char **argv)
{
	struct arguments ar;
	ar.verbose = 0;
	ar.new_game = 0;

	struct my_rand rnd;

	argp_parse(&my_argp, argc, argv, 0, 0, &ar);

	init_rand(&rnd, &ar);
	get_num(&rnd, &ar);
	
	if(ar.num == rnd.rand_num) rnd.score_stat++;
	
	fprintf(stdout, "%sYour number: %d\n", color(WHITE), ar.num);
	fprintf(stdout, "Computer number: %d\n", rnd.rand_num);
	fprintf(stdout, rnd.score_stat ? "%sYou guessed!\n" : "%sYou missed\n",
			rnd.score_stat ? color(GREEN) : color(YELLOW));

	save_data(&rnd, &ar);
	exit_rand(&rnd, &ar);	
	
	return 0;
}
