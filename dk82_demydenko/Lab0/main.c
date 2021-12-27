#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

uint32_t roll_dice(void)
{
    return rand() % 6 + 1;
}

int main(void)
{
    srand(time(NULL));

    uint32_t player_score = 0;
    uint32_t computer_score = 0;
    uint32_t player = 0;
    uint32_t computer = 0;
    char ch = '\n';

    while (ch != 'q') {
        player = roll_dice();
        computer = roll_dice();

        printf("Player`s dice :%d\nComputer`s dice :%d\n", player, computer);

        if (computer > player) {
            computer_score++;
            printf("Computer wins.\n");
        }
        else if (computer < player) {
            player_score++;
            printf("Player wins.\n");
        }
        else printf("Draw!\n");

        printf("Current score:\n\tComputer %d : Player %d\n", computer_score, player_score);
        printf("Press any key to retry or q to exit\n");

        ch = getch();
    }
    return 0;
}
