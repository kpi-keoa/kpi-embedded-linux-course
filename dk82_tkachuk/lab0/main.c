#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int value;
} player;

player user, computer;

int main(void)
{
    FILE *fp;
    fp = fopen("/dev/urandom", "r");
    char *name = malloc(sizeof(char) * 100);
    char *key = malloc(sizeof(char));

    printf("Enter your Name: ");
    scanf("%s", name);
    printf("Hello, %s !\n", name);

    while (1) {
        printf("Enter letter \"n\" to play or another symbol to exit\n");
        scanf("%s", key);

        if (key[0] == 'n') {
            fread(&user.value, sizeof(int), 1, fp);
            fread(&computer.value, sizeof(int), 1, fp);

            if (user.value > computer.value) {
                printf("\n%s win!\n", name);
            } else if (user.value < computer.value) {
                printf("\nComputer win!\n");
            } else {
                printf("\nParity!\n");
            }
        } else
            break;
    }
    free(name);
    free(key);
    fclose(fp);
    return 0;
}
