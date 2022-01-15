#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum PARSE_STATUS
{
        _PARSE_OK = 0,
        _PARSE_ERR= 1
};

enum ATOI_STATUS
{
        _ATOI_OK = 0,
        _ATOI_ERR = 1
};

// used by main to communicate with parse_opt
struct Parse_Args_Global
{
        int coordinates[256];
        int dist;
        int verbose;
        int size;
};

int addNewCoordinate(struct Parse_Args_Global *args, char *coordinate);

error_t parse_opt(int key, char *arg, struct argp_state *state);

void print_Parse_Args_Global(struct Parse_Args_Global *args);


