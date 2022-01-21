#include "utils.h"

int main(int argc, char *argv[]) {
    enum Verbose arg_verb = VERBOSE_DISABLE;
    enum Dist arg_dist = DIST_DISABLE;

    if (argc == 1) {
        fprintf(stderr, "%s", "No arguments!\n");
        view_help();
        return NO_ARGUMENTS;
    }

    // arguments parsing
    for (int i = 1; i < argc; i++) {
        if (strcmp("-v", argv[i]) == 0 || strcmp("--verbose", argv[i]) == 0) {
            arg_verb = VERBOSE_ENABLE;
            if (arg_verb)
                fprintf(stdout, "%s", "-v captured\n");
        }

        if (strcmp("--dist", argv[i]) == 0) {
            arg_dist = DIST_ENABLE;
            if (arg_verb)
                fprintf(stdout, "%s", "--dist captured\n");
        }

        if (strcmp("--help", argv[i]) == 0 || strcmp("-help", argv[i]) == 0
                || strcmp("help", argv[i]) == 0) {
            view_help();
            return EXECUTION_SUCCESSFUL;
        }
    }

    // unless another point coordinate is provided
    if ((argc - arg_verb - arg_dist) % 2 == 0) {
        fprintf(stderr, "%s", "Invalid point!\n");
        return INVALID_POINT;
    }

    // print argv and argc info
    if (arg_verb) {
        for (int i = 0; i < argc; i++)
            fprintf(stdout, "argv[%d] = %s\n", i, argv[i]);
        fprintf(stdout, "argc = %d\n", argc);
    }

    // create array of entered points
    Array *theArray = create_array((argc - arg_verb - arg_dist)/2 + 1); // + 1 for result point

    // filling the array
    for (int i = arg_verb + arg_dist + 1; i < argc; i += 2) {
        Point A = {atoi(argv[i]), atoi(argv[i+1])};
        add_element(theArray, &A);
    }

    equidistant_point(theArray);

    if (arg_verb) {
        print_array(theArray);
        fprintf(stdout, "Equidistant point and distance (if --dist is available):\n");
        fprintf(stdout, "%f %f\n", theArray->points[theArray->number - 1]->x,
                theArray->points[theArray->number - 1]->y);
        if (arg_dist)
            fprintf(stdout, "%f\n", theArray->distance);
    }
    else {
        fprintf(stdout, "%f %f\n", theArray->points[theArray->number - 1]->x,
                theArray->points[theArray->number - 1]->y);
        if (arg_dist)
            fprintf(stdout, "%f\n", theArray->distance);
    }

    // free memory
    free_array(theArray);

    return EXECUTION_SUCCESSFUL;
}
