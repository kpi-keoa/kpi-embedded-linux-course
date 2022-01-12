#include "utils.h"

time_dir check_files_inst;

enum Verbose arg_verb = VERBOSE_DISABLE;
enum Path arg_path = PATH_DISABLE;

int main(int argc, const char *argv[])
{
    //*************************
    // arg_param[0] - dir_path
    // arg[1] - cond_day
    // arg[2] - cond_hour
    // arg[3] - cond_min
    //*************************

    int arg_param [4] = {0};

    // elaborate argp
    for (int i = 1; i < argc; i++) {
        if (!strcmp("-v", argv[i]) || !strcmp("--verbose", argv[i])) {
            arg_verb = VERBOSE_ENABLE;
            if (arg_verb)
                fprintf(stdout, "-verbose enabled\n");
        }
        if (argv[i][0] == '.' || argv[i][0] == '/' || argv[i][0] == '~') {
            arg_path = PATH_ENABLE;
            arg_param[0] = i;
            if (arg_verb)
                fprintf(stdout, "-dir.path entered\n");
        }
    }

    // copy date and time from argp
    for (int i = 1 + arg_verb + arg_path; i < argc; i++) {
        arg_param[i - arg_verb - arg_path] = atoi(argv[i]);
    }

    // copy to struct
    copy_time_to_inst(&check_files_inst, arg_param[1],
                arg_param[2], arg_param[3]);

    if (arg_verb) {
        fprintf(stdout, "Enter condition:\nday:%i\n", check_files_inst.day);
        fprintf(stdout, "hour:%i\n", check_files_inst.hour);
        fprintf(stdout, "min:%i\n", check_files_inst.min);
    }

    // chect valid date it struct
    if (check_date_valid(&check_files_inst) == DATE_ERROR) {
        fprintf(stderr, "INVALID ENTERED DATE\n");
        copy_time_to_inst(&check_files_inst, 0, 0, 0);
    }

    // run ls
    if (!arg_path) {
        if (arg_verb)
            fprintf(stdout, "dir_path: ./ \n");
        ls(".", arg_verb);
    } else {
        if (arg_verb)
            fprintf(stdout, "dir_path: %s \n", argv[arg_param[0]]);
        ls(argv[arg_param[0]], arg_verb);
    }
    return EXECUTION_SUCCESSFUL;
}
