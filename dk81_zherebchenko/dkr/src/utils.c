#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <argp.h>
#include "utils.h"

error_t parse_opt(int key, char *arg, struct argp_state *state)
{
        struct arguments *arguments = state -> input;

        switch (key) {
        case 'v':
                arguments -> verbose = 1;
                break;
        case 'd':
                arguments -> dist = 1;
                break;
        case ARGP_KEY_ARG:
                arguments -> args[state -> arg_num] = arg;
                break;
        case ARGP_KEY_END:
                if (state -> arg_num < 4 || state -> arg_num % 2 != 0) {
                        fprintf(stderr, "Not enough arguments.\n", arg);
                        argp_usage(state);
                        break;
                }
        default:
                return ARGP_ERR_UNKNOWN;
        }
        return 0;
}

