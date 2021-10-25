#include "rnd.h"

int main(void)
{
	short temp;
	player human = { 0, 0 };
	player comp = { 0, 0 };

	while (1) {
		temp = space_get();
		if (temp == 1) {
			rnd_get(&human, &comp);
			print_res(&human, &comp);
		} else if (temp == 2) {
			break;
		}
	}
	return 0;
}
