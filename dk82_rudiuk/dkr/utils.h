#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
//#include <math.h>

enum Verbose {
    VERBOSE_DISABLE,
    VERBOSE_ENABLE
};

enum Set_bit {
    SB_DISABLE,
    SB_ENABLE
};

enum Set_bits {
    SBS_DISABLE,
    SBS_ENABLE
};

enum Unset_bit {
    USB_DISABLE,
    USB_ENABLE
};

enum Unset_bits {
    USBS_DISABLE,
    USBS_ENABLE
};

enum Flip_bit {
    FB_DISABLE,
    FB_ENABLE
};

enum Flip_bits {
    FBS_DISABLE,
    FBS_ENABLE
};

enum Radix_ {
    BINARY,
    DECIMAL,
    HEX
};

typedef struct Number_ {
    char *number_str;
    int number;
    enum Radix_ radix;
} Number;

enum Errors {
    EXECUTION_SUCCESSFUL,
    NO_ARGUMENTS,
    NOT_ENOUGH_ARGUMENTS,
    INVALID_DATA_INPUT,
    TOO_MANY_ARGUMENTS,
    MEMORY_ALLOCATION_ERROR
};

void free_number(Number *A);
void parse_arguments(int argc, char *argv[]);
void help_print(void);
void change_bits(Number *A, int argc, char *argv[]);
void create_number(Number *A, char *argv[]);
void number_prefix_delete(Number *A);
void print_number(Number *A);
int setbit(const int value, const int position);
int unsetbit(const int value, const int position);
int switchbit(const int value, const int position);
int is_number(const char *number);
