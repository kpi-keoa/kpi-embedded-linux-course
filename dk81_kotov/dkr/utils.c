#include "utils.h"



unsigned long file_size(char* fp1)
{
    FILE* f1 = fopen(fp1, "r+");

    if (f1 == NULL) {
        fprintf(stderr, "Something went wrong with the file\n");
        exit(FILE_FAIL);
    }

    fseek(f1, 0L, SEEK_END);
    unsigned long file_size = ftell(f1);

    fclose(f1);

    return file_size;
}

unsigned long byte_difference(char* fp1, char* fp2, unsigned long f1_size, unsigned long f2_size)
{
    unsigned long cnt_bytes = 0;
    char f1_byte, f2_byte;
    long difference_files = 0;
    unsigned long compare_sizes = 0;
    if (f1_size < f2_size) {
        compare_sizes = f1_size;
        difference_files = f2_size - f1_size;
    }
    else {
        compare_sizes = f2_size;
        difference_files = f1_size - f2_size;   
    }



    FILE* f1 = fopen(fp1, "r+");
    FILE* f2 = fopen(fp2, "r+");

    if (f1 == NULL || f2 == NULL) {
        fprintf(stderr, "Something went wrong with the file\n");
        exit(FILE_FAIL);
    }

    for (unsigned long i = 0; i < compare_sizes; i++)
    {
        fread(&f1_byte, sizeof(char), 1, f1);
        fread(&f2_byte, sizeof(char), 1, f2);
        if (f1_byte != f2_byte)
            cnt_bytes++;
    }

    fclose(f1);
    fclose(f2);
    
    if (f1_size == f2_size)
        return cnt_bytes;
    else return cnt_bytes + difference_files;
}

void trim_file(char* fp1, unsigned long trim_size)
{
    FILE* f1 = fopen(fp1, "r+");

    if (f1 == NULL) {
        fprintf(stderr, "Something went wrong with the file\n");
        exit(FILE_FAIL);
    }
    long trim_ok;
    trim_ok = ftruncate(fileno(f1), trim_size);

    fclose(f1);

}