#ifndef utils_h
#define utils_h

static struct argp_option options[] = {
        {"verbose",     'v',    0,      OPTION_ALIAS,    "Produce verbose output" }, 
        {"dist",        'd',    0,      OPTION_ALIAS,    "Distance to the equidistant point" }, 
        { 0 }
};

struct arguments {
        int verbose, dist;
        char **args;
};

error_t parse_opt(int key, char *arg, struct argp_state *state);

#endif /* utils_h */

