#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int randNum(void)
{
    uint8_t randomNum;
    FILE *fp = fopen("/dev/random", "r");
    fread(&randomNum, sizeof(randomNum), 1, fp);
    fclose(fp);
    return randomNum;
}

int main(int argc, char **argv)
{
    int dice[2];
    int player, computer;
    char choice;
    int i;

    printf("Dice Game\nPress any key to make a throw.");

    do {
        player = 0;
        computer = 0;
        getchar();
        for (i = 0; i < 2; i++) {
            dice[i] = 1+randNum()/(42+2/3);
            player += dice[i];
        }
        printf("\nYour throw: %d %d\n", dice[0], dice[1]);
        for (i = 0; i < 2; i++) {
            dice[i] = 1+randNum()/(42+2/3);
            computer += dice[i];
        }
        printf("Computer throw: %d %d\n", dice[0], dice[1]);

        if (player == computer) {
            printf("Tie!\n\n");
        }
        if (player > computer) {
            printf("You win! Your point total: %d. Computer point total: %d.\n\n", player, computer);
        }
        if (player < computer) {
            printf("The computer won! His point total: %d. Your point total: %d.\n\n", computer, player);
        }
        printf("Play it again? Y/N\n");
        choice = getchar();
    } while (choice == 'Y');
    return 0;
}
