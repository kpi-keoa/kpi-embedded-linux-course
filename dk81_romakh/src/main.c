/*
 *   Copyright (c) 2021
 *   All rights reserved.
 */

#include "rand.h"

int main(void)
{
	int result = GAME_OK;
	char button_pressed;

	while (1) {
		if (result == GAME_ERROR) {
			printf("Game Error!\n");
			break;
		} else {
			printf("Press enter to retry, q to exit\n");
			scanf("%c", &button_pressed);

			if (button_pressed == 'q') {
				break;
			}
			result = game();
		}
	}
	return 0;
}
