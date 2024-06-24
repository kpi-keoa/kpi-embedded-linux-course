#include <stdio.h>
#include <time.h>
#include <stdlib.h>
//#include <unistd.h>

#define PS_ANIM_SEQ_LENGTH 6
#define MAX_RND 9



enum GAME_STATUS{
                GAME_OK = 0,
                GAME_ERROR = 1
};

int game(void);
void init_sequence(int seq[], int seq_length);
int delay(long milli_seconds);
void pseudo_animation(const char *message, int seq[], int seq_length);
