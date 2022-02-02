#include "utils.h"

static char doc[] = "Calculate difference between files in bytes";

static char args_doc[] = "Input two file names to compare each of them byte by byte";

static struct argp_option options[] = {
    {"verbose", 'v', "VERBOSITY", OPTION_ARG_OPTIONAL, "Produce verbose output"},
    {"trim", 't', "TRIMMED", OPTION_ARG_OPTIONAL, "Trim a larger file to the size of a smaller one"},
    {0, 0, "FILE", 0, "File for reading"},
    {0}
};

static struct argp argp = { options, parse_opt, args_doc, doc };

int main (int argc, char **argv)
{
    //declare and intitialize structure to store parsed args
    struct arguments arguments;

    arguments.trim = 0;
    arguments.verbose = 0;
    arguments.index = 0;

    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    unsigned long file1_size = calculate_file_size(arguments.input_file_names[0]);
    unsigned long file2_size = calculate_file_size(arguments.input_file_names[1]);

    unsigned char is_first_bigger = (file1_size > file2_size) ? 1 : 0;

    if (arguments.verbose) {
        fprintf(stdout, "\n[%s] size is %u bytes and [%s] size is %u bytes\n",
            arguments.input_file_names[0], file1_size,
            arguments.input_file_names[1], file2_size);
    }

    if (arguments.trim) {
        if (is_first_bigger) {
            trim_file(arguments.input_file_names[0], file2_size);
            file1_size = file2_size;
        }
        else {
            trim_file(arguments.input_file_names[1], file1_size);
            file2_size = file1_size;
        }
        if (arguments.verbose) {
            fprintf(stdout, "After trimming [%s] size is %u bytes and [%s] size is %u bytes\n",
                arguments.input_file_names[0], file1_size,
                arguments.input_file_names[1], file2_size);
        }
    }

    fprintf(stdout, "Difference in bytes between files is : %u\n",
        compare_files(arguments.input_file_names[0], arguments.input_file_names[1]));

    exit(0);
}
