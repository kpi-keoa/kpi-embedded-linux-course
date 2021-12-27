#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned long long uint64;
typedef unsigned short uint16;
//ncurses
uint16 game()
{
	uint16 human;
	uint16 T1000;
	uint16 human_cnt = 0;
	uint16 T1000_cnt = 0;
	char drop = '\0';
	char _unused = '\0';
	while (human_cnt != 3 && T1000_cnt != 3) {
		printf("drop again?(d): ");
		if (1 != scanf("%c%c", &drop, &_unused)) {
			fprintf(stderr, "err\n");
			// error
		}
		if ('d' == drop) {
			human = rand() % 6 + 1;
			T1000 = rand() % 6 + 1;
			printf("your number is %d\n", human);
			printf("T1000's number is %d\n", T1000);
			if (human > T1000)
				human_cnt++;
			else if (T1000 > human)
				T1000_cnt++;
		} else
			printf("something went wrong\n");
	}
	if (human_cnt > T1000_cnt)
		return 1;
	else if (T1000_cnt > human_cnt)
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
		printf("You have to win 3 rounds to win the game\n");
		winner = game();
		if (winner == 1)
			printf("You won the game\n");
		else if (winner == 0)
			printf("You loose the game\n");
	} else
		printf("(%c)wrong, try again\n", start);

	return 0;
}