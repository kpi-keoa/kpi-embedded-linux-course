#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <argp.h>
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

enum TRIM_STATUS {
    __TRIM_OK__ = 0,
    __TRIM_FAIL__ = -1
};



struct arguments
{
    char input_file_names[2][20];
    unsigned long trim;
    unsigned long verbose;
    unsigned long index;
};

error_t parse_opt(int key, char *arg, struct argp_state *state);

unsigned long calculate_file_size(char* file_name);

enum TRIM_STATUS trim_file(char* file_name, unsigned long desired_size);

unsigned long compare_files(char* file_name1, char* file_name2);
