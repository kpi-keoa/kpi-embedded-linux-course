#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    int uservalue, computervalue;
    srand(time(NULL));
    char *name = malloc(sizeof(name) * 100);
    char *key = malloc(sizeof(key));

    printf("Enter your Name: ");
    scanf("%s", name);
    printf("Hello, %s !\n", name);

    while (1) {
        printf("Enter letter \"n\" to play or another symbol to exit\n");
        scanf("%s", key);

        if (key[0] == 'n') {

            uservalue = rand() %100 + 1;
            computervalue = rand() %100 + 1;

            if (uservalue > computervalue) {
                printf("\n%s win!\n", name);
            } else if (uservalue < computervalue) {
                printf("\nComputer win!\n");
            } else {
                printf("\nParity!\n");
            }
        } else
            break;
    }
    free(name);
    free(key);
    return 0;
}
