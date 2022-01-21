#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned long long uint64;
typedef unsigned short uint16;

uint16 game()
{
	uint16 your;
	uint16 enemy;
	uint16 your_cnt = 0;
	uint16 enemy_cnt = 0;
	char drop = '\0';
	char _unused = '\0';
	while (your_cnt != 2 && enemy_cnt != 2) {
		printf("drop again?(d): ");
		if (1 != scanf("%c%c", &drop, &_unused)) {
			fprintf(stderr, "err\n");
		}
		if ('d' == drop) {
			your = rand() % 6 + 1;
			enemy = rand() % 6 + 1;
			printf("your number is %d\n", your);
			printf("enemy's number is %d\n", enemy);
			if (your > enemy)
				your_cnt++;
			else if (enemy > your)
				enemy_cnt++;
		} else
			printf("something went wrong\n");
	}
	if (your_cnt > enemy_cnt)
		return 1;
	else if (enemy_cnt > your_cnt)
		return 0;
}

int main(void)
{
	srand(time(0));

	char start = '\0';
	uint16 winner = 0;
	char _unused = '\0';
	printf("To start the game press space: ");
	if (1 != scanf("%c%c", &start, &_unused)) {
		fprintf(stderr, "err\n");
		// error
	}
	if (' ' == start) {
		printf("you entered '%c'\n", start);
		printf("You have to win 2 rounds to win the game\n");
		winner = game();
		if (winner == 1)
			printf("You won the game\n");
		else if (winner == 0)
			printf("You loose the game\n");
	} else
		printf("(%c)wrong, try again\n", start);

	return 0;
}