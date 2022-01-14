#include "utils.h"

int main (int argc, char *argv[]){
    long first_file_size = 0;
    long second_file_size = 0;
    long delta = 0;
    char buff [100];
    int velosiped = 0;

    enum Verbose arg_verb = VERBOSE_DISABLE;
    enum Trim arg_trim = TRIM_DISABLE;

    for (int i = 1; i < argc; i++) {
        if (strcmp("-v", argv[i]) == 0 || strcmp("--verbose", argv[i]) == 0) {
            arg_verb = VERBOSE_ENABLE;
            if (arg_verb)
                fprintf(stdout, "%s", "-v captured\n");
        }

        if (strcmp("--help", argv[i]) == 0) {
            view_help();
            return 1;
        }

        if (strcmp("--trim", argv[i]) == 0) {
            arg_trim = TRIM_ENABLE;
        }
    }

    FILE *first_file_pointer;
    if ((first_file_pointer = fopen("first_file.txt","rt")) == NULL){
        printf("Cannot open required file\n");
        exit(1);
    }

    fseek(first_file_pointer, 0L, SEEK_END);
    first_file_size = ftell(first_file_pointer);
    printf("The first file length is %ld bytes long.\n", first_file_size);

    fclose(first_file_pointer);

    FILE *second_file_pointer;
    if ((second_file_pointer = fopen("second_file.txt","rt")) == NULL){
        printf("Cannot open required file\n");
        exit(1);
    }

    fseek(second_file_pointer, 0L, SEEK_END);
    second_file_size = ftell(second_file_pointer);
    printf("The second file length is %ld bytes long.\n", second_file_size);

    fclose(second_file_pointer);

    if (arg_trim == TRIM_ENABLE){
        if (first_file_size == second_file_size) {
            printf("Our files are equal by size\n");
        } else if (first_file_size < second_file_size) {
            printf("Second file is bigger than first\n");
            delta = second_file_size - first_file_size;

            FILE *second_file_pointer;
            if ((second_file_pointer = fopen("second_file.txt","rt")) == NULL){
                printf("Cannot open required file\n");
            exit(1);
            }

            fseek(second_file_pointer, 0L, SEEK_SET);
            for (int i = 0; i = second_file_size; i++){
                buff [i] = fgetc(second_file_pointer);
            }
            fclose(second_file_pointer);

            //FILE *second_file_pointer;
            if ((second_file_pointer = fopen("second_file.txt","w")) == NULL){
                printf("Cannot open required file\n");
            exit(1);
            }

            fseek(second_file_pointer, 0L, SEEK_SET);
            velosiped = second_file_size - delta;
            for (int j = 0; j = velosiped; j++) {
                fputc(buff[j], second_file_pointer);
            }
            printf("Our files are equal now\n");
        } else if (first_file_size > second_file_size) {
            printf("First file is bigger than second\n");
            delta = first_file_size - second_file_size;

            //FILE *first_file_pointer;
            if ((first_file_pointer = fopen("first_file.txt","rt")) == NULL){
                printf("Cannot open required file\n");
            exit(1);
            }

            fseek(first_file_pointer, 0L, SEEK_END);
            for (int i = 0; i = first_file_size; i++){
                buff [i] = fgetc(first_file_pointer);
            }
            fclose(first_file_pointer);

            //FILE *second_file_pointer;
            if ((first_file_pointer = fopen("first_file.txt","w")) == NULL){
                printf("Cannot open required file\n");
            exit(1);
            }

            fseek(first_file_pointer, 0L, SEEK_SET);
            velosiped = first_file_size - delta;
            for (int j = 0; j = velosiped; j++) {
                fputc(buff[j], first_file_pointer);
            }
            printf("Our files are equal now\n");
        }
    }

    return 0;
}
