#include <stdio.h>
#include <stdint.h>
#include <time.h>
#define RANDOM rand()%6+1

int main()
{
	system("pause");
	srand(time(NULL));
	
	for(;;)
	{
		uint8_t player = 0, computer = 0;
		for (uint8_t i = 0; i < 3; i++) {
			uint8_t a = RANDOM, b = RANDOM;
			while (a == b)
			{
				printf("Player - %i, \nComputer - %i \n", a, b);
				printf("Tie! Reroll\n\n");

				a = RANDOM;
				b = RANDOM;

				system("pause");
			}
			printf("Player - %i, \nComputer - %i \n", a, b);
			printf(a > b ? "Player gets point!\n\n" : "Computer gets point!\n\n");
			if (a > b) player++;
			else if (a < b) computer++;
			system("pause");
		}
		printf(player> computer?"Player wins!\n":player== computer ?"Tie,drink bear,friends!\n":"Computer wins!\n");
		system("pause");
		system("cls");
	}
}