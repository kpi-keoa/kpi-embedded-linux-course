#include "../inc/utils.h"
#include <argp.h>
#include <stdlib.h>

const char *argp_program_version = "db 1.0";

const char *argp_program_bug_address = "<zheka.nysh@gmail.com>";

static char doc[] = "db --Simple database for storing records in a file.";

/* A description of the arguments we accept. */
static char args_doc[] = "";

static struct argp_option options[] = {
	{"add",      'a', "string", 0,  "Add person" },
	{"remove",   'r', "int",    0,  "Remove person" },
	{"list",     'l', 0,        0,  "List people" },
	{"verbose",  'v', 0,        0,  "Produce verbose output" },
	{"file",     'f', "FILE",   0,  "FILE database. Defaut name \033[1;1mbase.csv\033[0m" },
	{ 0 }
};

struct arguments {
	char *add;
	int remove, person, list, verbose;
	char *file;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch (key) {
		case 'a':
			arguments->add = arg;
			break;
		case 'r':
			arguments->remove = 1;
			sscanf(arg, "%d", &arguments->person);
			break;
		case 'l':
			arguments->list = 1;
			break;
		case 'v':
			arguments->verbose = 1;
			break;
		case 'f':
			arguments->file = arg;
			break;
		 default:
			return ARGP_ERR_UNKNOWN;
			break;
	}
	return 0;
}

/* argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };


enum STATE add_person(struct list_t *list, char *buf)
{
	enum STATE state;
	struct database *new_node;

	new_node = malloc(sizeof(*new_node));
	if (NULL == new_node)
		return STATE_EFA;

	state = line_to_struct(buf, new_node);
	if (STATE_EFC == state)
		return state;

	list_add_tail(&new_node->list, list);

	return STATE_OK;
}

enum STATE remove_person(struct list_t *list, int number)
{
	struct database *temp;
	struct list_t *pos;
	struct list_t *t;
	int i = 0;

	list_for_each_safe(pos, t, list) {
		temp = container_of(pos, struct database, list);

		if (i == number) {
			list_del(pos);
			free_database(temp);
			return STATE_OK;
		}
		i++;
	}

	return STATE_EEM;
}

void list_people(struct list_t *list, int verbose)
{
	struct database *temp;
	struct list_t *pos;

	int i = 0;

	if (verbose && list_empty(list)) {
		printf("Database is empty.\n");
		return;
	} else if (verbose)
		printf("№: Name Suname Patronymic Salary\n");

	list_for_each(pos, list) {
		temp = container_of(pos, struct database, list);
		if (verbose)
			printf("%d: ", i++);
		printf("%s,%s,%s,%d\n", temp->name, temp->suname,
				temp->patronymic, temp->salary);
	}
}

enum STATE read_database(char *filename, struct list_t *list)
{
	enum STATE state;
	char *buf;
	FILE *file;
	size_t SIZE = 300;

	state = STATE_OK;

	file = fopen(filename, "r");
	if (NULL == file) {
		state = STATE_EOP;
		goto final;
	}

	buf = malloc(sizeof(*buf) * SIZE);
	if (NULL == buf) {
		state = STATE_EFA;
		goto dealloc_file;
	}

	rewind(file);

	while(-1 != getline(&buf, &SIZE, file)) {
		state = add_person(list, buf);
		if (STATE_OK != state)
			break;
	}

	free(buf);
dealloc_file:
	fclose(file);
final:
	return state;
}

enum STATE write_database(char *filename, struct list_t *list)
{
	struct database *temp;
	struct list_t *pos;

	FILE *file = fopen(filename, "w");
	if (NULL == file)
		return STATE_EOP;

	list_for_each(pos, list) {
		temp = container_of(pos, struct database, list);
		fprintf(file, "%s,%s,%s,%d\n", temp->name, temp->suname,
					temp->patronymic, temp->salary);
	}

	fclose(file);
	return STATE_OK;
}

int main(int argc, char **argv)
{
	enum STATE state;
	struct database *temp;
	struct list_t *pos;
	struct list_t *t;
	struct arguments arguments;
	struct list_t list = LIST_T_INIT(list);

	/* Set arguments defaults */
	arguments.add = NULL;
	arguments.remove = 0;
	arguments.list = 0;
	arguments.file = "Base.csv";

	argp_parse (&argp, argc, argv, 0, 0, &arguments);

	/*
	 *   read file
	 */
	state = read_database(arguments.file, &list);
	if (STATE_OK != state) {
		print_error(state, arguments.verbose);
		goto end;
	}

	if (argc == 1)
		arguments.list = 1;

	if (NULL != arguments.add) {
		state = add_person(&list, arguments.add);
		if (STATE_OK != state) {
			print_error(state, arguments.verbose);
			goto dealloc_list;
		}
	}

	if (arguments.remove) {
		state = remove_person(&list, arguments.person);
		if (STATE_OK != state) {
			print_error(state, arguments.verbose);
			goto dealloc_list;
		}
	}

	if (arguments.list)
		list_people(&list, arguments.verbose);

	state = write_database(arguments.file, &list);
	if (STATE_OK != state) {
		print_error(state, arguments.verbose);
		goto dealloc_list;
	}

dealloc_list:
	list_for_each_safe(pos, t, &list) {
		temp = container_of(pos, struct database, list);

		list_del(pos);
		free_database(temp);
	}
end:
	exit(state);
}
