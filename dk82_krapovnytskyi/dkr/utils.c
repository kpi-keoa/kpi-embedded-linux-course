#include "utils.h"


error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key) {
        case 'v':
            arguments->verbose = 1;
            break;
        case 't':
            arguments->trim = 1;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num > 1) {
                argp_failure (state, 1, 0, "Too many arguments\n");
                argp_usage (state);
            }
            strcpy(arguments->input_file_names[arguments->index], arg);
            (arguments->index)++;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 2) {
                argp_failure (state, 1, 0, "Too few arguments\n");
                argp_usage (state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}


unsigned long calculate_file_size(char* file_name)
{
    FILE* file = fopen(file_name, "r+");

    if (file == NULL) {
        fprintf(stderr, "Incorrect file name or it doesn't exit\n");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0L, SEEK_END);
    unsigned long file_size = ftell(file);

    fclose(file);

    return file_size;
}

enum TRIM_STATUS trim_file(char* file_name, unsigned long desired_size)
{
    enum TRIM_STATUS trim_status = __TRIM_OK__;

    FILE* file = fopen(file_name, "r+");

    if (file == NULL) {
        fprintf(stderr, "Incorrect file name or it doesn't exit\n");
        exit(EXIT_FAILURE);
    }

    trim_status = ftruncate(fileno(file), desired_size);

    fclose(file);

    return trim_status;
}

unsigned long compare_files(char* file_name1, char* file_name2)
{

    char temp1, temp2;

    unsigned long different_bytes = 0;

    unsigned long file1_size = calculate_file_size(file_name1);
    unsigned long file2_size = calculate_file_size(file_name2);

    FILE* file1 = fopen(file_name1, "r+");
    FILE* file2 = fopen(file_name2, "r+");

    for (size_t i = 0; i < MIN(file1_size, file2_size); i++) {
        fread(&temp1, 1, 1, file1);
        fread(&temp2, 1, 1, file2);
        if (temp1 != temp2) different_bytes++;
    }

    fclose(file1);
    fclose(file2);

    if (file1_size == file2_size) return different_bytes;
    else return (different_bytes + abs((long) file1_size - (long) file2_size));
}
