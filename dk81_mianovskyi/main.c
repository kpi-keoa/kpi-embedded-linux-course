#include "rnd.h"

int main(void)
{
    int val;
    val = rand_value();
    draw_dice(val);
    return 0;
}
