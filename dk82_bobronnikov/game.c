#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int rolldice(int *scoretable, int playercount)
{
    int max = 0, winner = 0;
    int* dicetable = malloc(sizeof(dicetable) * playercount);
    for (int i = 0; i < playercount; i++) {
        dicetable[i] = rand() % 6 + 1;
        if (dicetable[i] > max) {
            winner = i;
            max = dicetable[i];
        }
    }
    printf("Winner: %d\n", winner);
    free(dicetable);
    scoretable[winner]++;
    return 0;
}

int display_scores(int* scoretable, int playercount)
{
    printf_s("*** Statistics: ***\n");
    for (int i = 0; i < playercount; i++) {
        printf_s("Player %d: %d\n", i, scoretable[i]);
    }
    printf_s("\n");
    return 0;
}

int nullscore (int *scoretable, int *playercount)
{
    for (int i = 0; i < playercount; i++) {
        scoretable[i] = 0;
    }
    return 0;
}

int main(int argv, char *argc[])
{
    srand(time(NULL));
    int playercount = 0;
    printf_s("How many players? ");
    scanf_s("%d", &playercount);
    printf_s("\n");
    int *scoretable = malloc(sizeof(scoretable) * playercount);
    nullscore(&scoretable, playercount);
    while (1) {
        getch();
        rolldice(&scoretable, playercount);
        display_scores(&scoretable, playercount);
    }
    return 0;
}
