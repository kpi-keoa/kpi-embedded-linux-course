#include "rnd.h"

void rnd_get(player *human, player *comp)
{
	human->player_point = 0;
	comp->player_point = 0;

	srand(time(NULL));
	for (int i = 0; i < 3; i++) {
		human->cube_value = rand() % 6 + 1;
		comp->cube_value = rand() % 6 + 1;
		printf("\tTry №%d\n\tHuman: %d \n\tComp: %d\n", i + 1,
		       human->cube_value, comp->cube_value);
		if (comp->cube_value > human->cube_value) {
			comp->player_point++;
			printf("Point for COMP\n\n");
		} else {
			human->player_point++;
			printf("Point for HUMAN\n\n");
		}
	}
}

void print_res(player *human, player *comp)
{
	if (comp->player_point > human->player_point) {
		printf("General points: HUMAN (%d), COMP (%d). Comp winner!!!\n",
		       human->player_point, comp->player_point);
	} else {
		printf("General points: HUMAN (%d), COMP (%d). Human winner!!!\n",
		       human->player_point, comp->player_point);
	}
}

// int flushbufBoard(void)
// {
//         int ch;
//
//         while ((ch = getchar()) != EOF || ch != '\n') {
//         if (ch == EOF) {
//             if (feof(stdin)) {
//                 }
//
//             }
//             else {
//                 /* Handle stdin error. */
//             }
//         }
// }

enum OUT_STATE space_get(void)
{
	int ch;
	char buf[BUFSIZ];

	while ((ch = getchar()) != '\n' && ch != EOF);
	printf("Press SPACE key throwing dice: ");
	fgets(buf, sizeof(buf), stdin);
	if (buf[0] == 32) {
		return out_ok;
	} else if ((buf[0] == 113) || (buf[0] == 81)) {
		printf("EXIT\n");
		return out_stop;
	} else {
		printf("FALSE KEY\n");
		return out_false;
	}
}
