#include "utils.h"

int main(int argc, char *argv[]) {
    enum Verbose arg_verb = VERBOSE_DISABLE;
    enum TRIM arg_trim = TRIM_DISABLE;
    unsigned long file_check = 0;
    char file1[] = "file1.txt";
    char file2[] = "file2.txt";
    if (argc == 1) {
        fprintf(stderr, "%s", "No arguments!\n");
        return NO_ARGUMENTS;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp("-v", argv[i]) == 0 || strcmp("--verbose", argv[i]) == 0) {
            arg_verb = VERBOSE_ENABLE;
            if (arg_verb)
                fprintf(stdout, "%s", "-v captured\n");
        }

        if (strcmp("--trim", argv[i]) == 0) {
            arg_trim = TRIM_ENABLE;
            if (arg_verb)
                fprintf(stdout, "%s", "--trim captured\n");
        }
    }

    unsigned long f1_size = file_size(file1);
    unsigned long f2_size = file_size(file2);
    if (f1_size > f2_size)
        file_check = 1;
    else 
        file_check = 0;

    if (arg_verb) {
        fprintf(stdout, "first file size is %u bytes and second file size is %u bytes\n", 
            f1_size, f2_size);
        fprintf(stdout, "difference in files is %u bytes\n", byte_difference(file1, file2, f1_size, f2_size));
    }
    else {
        fprintf(stdout, "%u %u\n", f1_size, f2_size);
        fprintf(stdout, "%u\n", byte_difference(file1, file2, f1_size, f2_size));
    }
    
    if (TRIM_ENABLE) {
        if(file_check == 1) {
            trim_file(file1, f2_size);
            f1_size = f2_size;
        }
        else {
            trim_file(file2, f1_size);
            f2_size = f1_size;
        }
        if (arg_verb) 
            if (file_check == 1)
                fprintf(stdout, "first file was trimed to size %u\n", f2_size);
            else
                fprintf(stdout, "second file was trimed to size %u\n", f1_size);    
        else
            if (file_check == 1)
                fprintf(stdout, "first file was trimed to size %u\n", f2_size);
            else
                fprintf(stdout, "second file was trimed to size %u\n", f1_size);

    }

    return EXECUTION_SUCCESSFUL;
}