#include <stdio.h>
#include <time.h>
#include <stdlib.h>

typedef struct {
	int amount_wins;
	int value;
} player;

player user, comp;

int main(void)
{
	char key;
	char *name;
	//srand(time(NULL));
	printf("Enter your Name: ");
	scanf("%s", name);
	printf("Hello, %s !\n", name);

	while (1) {
		printf("Enter space-key");
		scanf("%c", &key);
		if (key == ' ') {
			user.value = rand() % 100 + 1;
			comp.value = rand() % 100 + 1;

			user.value > comp.value ? printf("%s win!", name) :
							printf("Computer win");
		}
	}
	return 0;
}
