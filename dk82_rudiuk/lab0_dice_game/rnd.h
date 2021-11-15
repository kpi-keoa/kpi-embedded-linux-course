#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum OUT_STATE { out_false = 0, out_ok = 1, out_stop = 2 };

typedef struct player_s {
	int player_point;
	int cube_value;
} player;

enum OUT_STATE space_get(void);
void rnd_get(player *human, player *comp);
void print_res(player *human, player *comp);
