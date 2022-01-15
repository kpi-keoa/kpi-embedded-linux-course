#include "../inc/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum STATE cp(char *string1, char **string2, size_t size)
{
	unsigned long i;
	char *buf;

	buf = malloc(sizeof(*buf) * size);
	if (NULL == buf)
		return STATE_EFA;

	for (i = 0; i < size; i++)
		buf[i] = string1[i];
	buf[size - 1] = '\0';
	*string2 = buf;

	return STATE_OK;
}

char *char_to_character(char *addr_begin, int characters, char **string)
{
	size_t size;
	char *addr_character;

	addr_character = strchr(addr_begin, characters);
	if (addr_character) {
		size = addr_character - addr_begin + 1;
		cp(addr_begin, string, size);
	}

	return addr_character;
}

enum STATE line_to_struct(char *line, struct database *new_node)
{
	char *addr_character;
	char *number;

	addr_character = char_to_character(line, ',', &new_node->name);
	if (0 == addr_character)
		return STATE_EFC;
	addr_character = char_to_character(++addr_character, ',',
					   &new_node->suname);
	if (0 == addr_character) {
		free(new_node->name);
		return STATE_EFC;
	}
	addr_character = char_to_character(++addr_character, ',',
					   &new_node->patronymic);
	if (0 == addr_character) {
		free(new_node->name);
		free(new_node->suname);
		return STATE_EFC;
	}
	addr_character = char_to_character(++addr_character, '\0', &number);
	if (0 == addr_character) {
		free(new_node->name);
		free(new_node->suname);
		free(new_node->patronymic);
		return STATE_EFC;
	}
	sscanf(number, "%d", &new_node->salary);

	return STATE_OK;
}

void free_database(struct database *str)
{
	free(str->name);
	free(str->suname);
	free(str->patronymic);
	free(str);
}

void print_error(enum STATE state, int verbose)
{
	switch (state) {
		case STATE_EOP:
			if (verbose)
				fprintf(stderr, "Error: ");
			fprintf(stderr, "[EOP] The file cannot be opened. "
					"The file name is incorrect or the"
				        " file does not exist.\n");
			break;
		case STATE_EFA:
			if (verbose)
				fprintf(stderr, "Error: ");
			fprintf(stderr, "[EFA] The function could not "
					"allocate the requested memory"
					"block.\n");
			break;
		case STATE_EFC:
			if (verbose)
				fprintf(stderr, "Error: ");
			fprintf(stderr, "[EFC] Data entered incorrectly.\n");
			break;
		case STATE_EEM:
			if (verbose)
				fprintf(stderr, "Error: ");
			fprintf(stderr, "[ERR] Record not found in database.");
			if (verbose)
				fprintf(stderr, " The number is out of range.");
			fprintf(stderr, "\n");
			break;
		default:
			break;
	}
}

