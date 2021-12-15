#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
	srand(time(0));
	char play = '\0';
	char _unused = '\0';
	unsigned long player = 0;
	unsigned long computer = 0;
	unsigned long count = 3;
	unsigned long player_score = 0;
	unsigned long computer_score = 0;
	for (unsigned long i = 1; i <= count; ++i) {
		printf("play the game?(y) \n");
		//scanf("%c", &play);
		if (1 != scanf("%c%c", &play, &_unused)) {
			//fprintf(stderr, "err\n");
			// error
		}
		if (play != 'y') {
			printf("You entered something wrong. Exit programm\n");
			return 0;
		} else {
			player = rand() % 6 + 1;
			computer = rand() % 6 + 1;
			printf("Computer = %ld\n", computer);
			printf("Player = %ld\n", player);
			if (computer > player) {
				printf("In round %ld won computer\n", i);
				computer_score++;
			} else if (player > computer) {
				printf("In round %ld won player\n", i);
				player_score++;
			} else
				printf("In round %ld nobody won\n", i);
		}
	}

	if (player_score > computer_score)
		printf("Player won with score %ld:%ld\n", player_score,
		       computer_score);
	else if (computer_score > player_score) {
		printf("Computer won with score %ld:%ld\n", computer_score,
		       player_score);
	} else
		printf("nobody won\n");

	return 0;
}