#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>


typedef struct {
    int day;
    int hour;
    int min;
} time_dir;

enum Verbose {
    VERBOSE_DISABLE,
    VERBOSE_ENABLE
};

enum Path {
    PATH_DISABLE,
    PATH_ENABLE
};

enum Errors {
    EXECUTION_SUCCESSFUL=0,
    PATH_ERROR,
    DATE_ERROR
};

void ls(const char *dir, enum Verbose arg_verb);
int check_file_time(time_dir* condition, struct tm* check_file);
void copy_time_to_inst (time_dir* check_file, int day, int hour, int min);
enum Errors check_date_valid (time_dir* inst);
