#include "utils.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <argp.h>

struct arguments {
    uint32_t guess;
    uint32_t boundary;
    gflags_t flags;
};
 
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *args = state->input;
        
    switch (key)
    {
    case 'n':
        args->boundary = atoi(arg);
        args->flags.is_newgame = true;
        args->flags.is_loadgame = false;
        
        if (args->boundary <= 1)
            argp_usage(state);
        
        break;
    case 't':
        args->guess = atoi(arg);
        args->flags.is_newgame = false;
        args->flags.is_loadgame = true;
        
        if (args->guess == 0)
            argp_usage(state);
            
        break;
        
    case 'v':
        args->flags.is_verbose = true;
            
        break;
        
    /* argc checking */
    case ARGP_KEY_ARG:
        if (state->argc >= 2)
            argp_usage(state);
        break;
        
    case ARGP_KEY_END:
        if (state->argc < 2)
            argp_usage(state);
        break;
        
    default:
        return ARGP_ERR_UNKNOWN;
        break;
    };
    
    return 0;
}

/* main */
int main(int argc, char **argv)
{
    /* return value */
    int retval = GAME_EOK;

    /* available options */
    struct argp_option options[] = {
        { "verbose", 'v', 0, 0, "Produce verbose output" },
        { "new", 'n', "NUM", 0, "Start a new game, with set boundary <NUM> > 1" },
        { "try", 't', "NUM", 0, "Play the game by guessing with <NUM> > 0" },
        { 0 }
    };
    
    /* argp struct */
    struct argp argp = {
        options, parse_opt, 0, 0
    };
    
    /* container for parsed arguments */
    struct arguments game_vars = {
        0, 0,
        { false, false, false }
    };
        
    /* parse argv and set flags */
    argp_parse(&argp, argc, argv, 0, 0, &game_vars);
    
    /* start game */
    game_t *game = game_create();
    game->flags = game_vars.flags;  // set only after parse
    
    if (game->flags.is_newgame && !game->flags.is_loadgame) {
        retval = game_init(game, game_vars.boundary, GAME_NEW);
    } else if (!game->flags.is_newgame && game->flags.is_loadgame) {
        retval = game_init(game, game_vars.boundary, GAME_LOAD);
        
        if (game->status == GAME_EOK)
            retval = game_play(game, game_vars.guess); 
    } else {
        retval = GAME_EARG;
        if (game->flags.is_verbose)
            fprintf(stderr, "Invalid option given\n");
        else
            fprintf(stderr, "ERR: %d\n", retval);
    }
    
    game_free(game);
    
    return retval;
}
