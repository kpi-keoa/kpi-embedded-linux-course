#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define red "\e[31;1m"
#define blue "\e[34;1m"
#define green "\e[32;1m"
#define cyan "\e[36;1m"
#define yellow "\e[33;1m"
#define magenta "\e[35;1m"

int rand_value(void);
void draw_dice(int num);
