#include "rand.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* main */
uint32_t clamp_ui32(uint32_t val, uint32_t min, uint32_t max)
{
    const uint32_t ret = (val < min) ? min : val;
    return (ret > max) ? max : ret;
}

int main(int argc, char **argv)
{
    rnd_t sys_rand;
    rnd_init(&sys_rand, RND_TYPE_URANDOM);

    uint32_t plr_roll = 1 + srnd_get32(&sys_rand) % 6;    // player dice roll
    uint32_t eai_roll = 1 + srnd_get32(&sys_rand) % 6;    // enemy ai dice roll

    // lying to the user or having them lie to us
    if (argc > 1) {
        printf("Rolling for computer...\n");
        plr_roll = clamp_ui32(atoi(argv[1]), 1, 6);
    } else {
        printf("Rolling for human and computer...\n");
    }

    // choosing who has won
    printf("Human rolled %u, computer rolled %u.\n", plr_roll, eai_roll);

    if (plr_roll > eai_roll) {
        printf("Human won!\n");
    } else if (plr_roll < eai_roll) {
        printf("Computer won!\n");
    } else {
        printf("A draw!\n");
    }

    rnd_free(&sys_rand, RND_TYPE_URANDOM);
    return 0;
}
