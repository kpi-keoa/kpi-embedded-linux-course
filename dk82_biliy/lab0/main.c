#include "utils.h"
#include <argp.h>
#include <unistd.h>
#include <fcntl.h>

const char *argp_program_bug_address = "vitaliy2034v@gmail.com";
const char *argp_program_version 	 = "Version 1.0b";

#define MAX_DICE_CNT	5

typedef struct {
	uint32_t 	dice_count;
	bool		no_graphics;
} control_block_t;

static error_t argp_parser(int __key, char *__arg, struct argp_state *__state)
{
	control_block_t *cb = ((control_block_t *)__state->input);
	switch(__key)
	{

		case 'n':
			if(get_digit_from_text(__arg, &(cb->dice_count)) != UTIL_SUCCESS)
				argp_failure(__state, 1, 0, "Error in converting argument: %s\n", __arg);

			if(cb->dice_count == 0 || cb->dice_count > MAX_DICE_CNT)
				argp_failure(__state, 1, 0, "Error argument invalid: %s\n", __arg);

			break;

		case 'g':
			cb->no_graphics = true;
			break;
		case ARGP_KEY_ARG:
			argp_failure(__state, 1, 0, "Unknown argument: %s\n", __arg);
			break;
	}
	return  0;
}


int main (int argc, char **argv)
{
	struct argp_option options[] = {
		{
			.arg   = "DIGIT",
			.doc   = "Count of dice. Values could be from 1 to 5",
			.flags = 0,
			.group = 0,
			.key   = 'n',
			.name  = "count"
		},
		{
			.arg   = NULL,
			.doc   = "Don`t show graphics",
			.flags = 0,
			.group = 0,
			.key   = 'g',
			.name  = "no-graphics"
		},
		{0}
	};
	struct argp argp_test  = {options, argp_parser, 0, "This program can generate dice"};

	control_block_t cb = {1, false};
	char rand_buff[MAX_DICE_CNT] = {0};
	int file_desc 	   = -1;


	//Check that parameters and options correctly recived
    error_t err_code = argp_parse(&argp_test, argc, argv, 0, 0, &cb);

    if(err_code)
    {
    	fprintf(stderr, "dice: Parse error: %s", strerror(err_code));
    	exit(err_code);
    }



    file_desc = open("/dev/urandom", O_RDONLY);
    if(file_desc == -1)
    {
    	fprintf(stderr, "dice: Error could not open /dev/urandom");
    	exit(-1);
    }
    if(read(file_desc, rand_buff, MAX_DICE_CNT) != 8)
    {
    	fprintf(stderr, "dice: Error in read /dev/urandom");
    	close(file_desc);
    	exit(-1);
    }
    close(file_desc);

   if(cb.no_graphics)
   {
	   for (int i = 0; i < cb.dice_count; ++i)
		   printf("%d", (rand_buff[i]%6)+1);
   }
   else
   {
	   for (int i = 0; i < cb.dice_count; ++i)
	   {
		   if(printf_dice((rand_buff[i]%6)+1) != UTIL_SUCCESS)
		   {
			   fprintf(stderr, "dice: Error could not print dice");
               exit(-1);
           }
	   }
   }

   printf("\n");
   exit(0);
}
