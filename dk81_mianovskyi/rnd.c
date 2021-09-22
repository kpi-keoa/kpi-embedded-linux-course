#include "rnd.h"

int rand_value(void)
{
    int r_value;
    unsigned int seed;
    seed = (unsigned int)(time(NULL) / 2);
    srand(seed);
    r_value = rand();
    r_value %= 6;
    return r_value;
}

void set_color(int seed)
{
    switch (seed) {
    case 0:
        printf(red);
        break;
    case 1:
        printf(blue);
        break;
    case 2:
        printf(green);
        break;
    case 3:
        printf(cyan);
        break;
    case 4:
        printf(yellow);
        break;
    case 5:
        printf(magenta);
        break;
    }
}

void draw_dice(int num)
{
    set_color(num);
    printf("#########\n");
    printf("#       #\n");
    switch (num) {
    case 0:
        printf("#       #\n");
        printf("#       #\n");
        printf("#   #   #\n");
        printf("#       #\n");
        printf("#       #\n");
        break;
    case 1:
        printf("# #     #\n");
        printf("#       #\n");
        printf("#       #\n");
        printf("#       #\n");
        printf("#     # #\n");
        break;
    case 2:
        printf("# #     #\n");
        printf("#       #\n");
        printf("#   #   #\n");
        printf("#       #\n");
        printf("#     # #\n");
        break;
    case 3:
        printf("# #   # #\n");
        printf("#       #\n");
        printf("#       #\n");
        printf("#       #\n");
        printf("# #   # #\n");
        break;
    case 4:
        printf("# #   # #\n");
        printf("#       #\n");
        printf("#   #   #\n");
        printf("#       #\n");
        printf("# #   # #\n");
        break;
    case 5:
        printf("# #   # #\n");
        printf("#       #\n");
        printf("# #   # #\n");
        printf("#       #\n");
        printf("# #   # #\n");
        break;
    }
    printf("#       #\n");
    printf("#########\n");
}