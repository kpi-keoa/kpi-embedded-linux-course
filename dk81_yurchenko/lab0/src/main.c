#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

const char file_random[] = "/dev/random";

enum FILE_STATE {
	FILE_OK = 0,
	FILE_EOP = 1,
	FILE_ERE = 2,
	FILE_EOF = 3
};

enum FILE_STATE init_rand(FILE **file)
{
	if (NULL == file)
		return FILE_EOP;

	*file = fopen(file_random, "r");

	if (NULL == file)
		return FILE_EOP;

	return FILE_OK;
}

enum FILE_STATE close_rand(FILE *file)
{
	int state;

	state = fclose(file);

	return state ? FILE_EOF : FILE_OK;
}

enum FILE_STATE read_rand(FILE *file, uint8_t *byte)
{
	size_t state;
	const size_t nmemb = 1;

	state = fread(byte, sizeof(*byte), nmemb, file);

	if (nmemb == state)
		return FILE_OK;

	return FILE_ERE;
}

int main(int argc, char **argv)
{
	FILE *file;
	uint8_t numb_pc, numb_my;
	enum FILE_STATE state;

	state = init_rand(&file);

	if (FILE_OK != state) {
		printf("ERROR\n");
		return 0;
	}

	printf("The cube is octagonal\n");

	read_rand(file, &numb_pc);
	numb_pc = numb_pc / 32 + 1;

	printf("PC throws dice: %i\n", numb_pc);

	read_rand(file, &numb_my);
	numb_my = numb_my / 32 + 1;

	printf("You throws dice: %i\n", numb_my);

	if (numb_my > numb_pc)
		printf("YOU WIN\n");
	else if (numb_my == numb_pc)
		printf("FRIENDSHIP WON\n");
	else
		printf("PC WIN\n");

	state = close_rand(file);

	if (FILE_OK != state) {
		printf("ERROR\n");
		return 0;
	}

	exit(0);
}

