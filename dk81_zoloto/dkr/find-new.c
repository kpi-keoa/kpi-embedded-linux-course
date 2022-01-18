#include <stdio.h>
#include <getopt.h>
#include <argp.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> 
#include "utils.h"


static const char *short_options = "vp:d:";

static struct option long_options[] = {
    { "verbose",       no_argument, NULL, 'v' },
    { "path",    required_argument, NULL, 'p' },
    { "date",    required_argument, NULL, 'd' },
    { NULL, 0, NULL, 0 } // last element must be filled with zeros
};

static struct {
    int verbose;
    char *path;
    char *date;
} arguments;

void parse_arguments(int argc, char *argv[]) {
    int opt;
    int option_index = 0;
	while ((opt = getopt_long(argc, argv, short_options,
		                      long_options, &option_index)) != -1) {
		switch(opt) {
            case 'p':
                arguments.path = optarg;
                break;
            case 'v':
                arguments.verbose = 1;
				break;
            case 'd':
                arguments.date = optarg;
		}
	}
}


enum Errors {
    OK = 0,
    DATE_REQUIRED = 1,
    PATH_REQUIRED = 2,
    DATE_INVALID = 3,
    PATH_INVALID = 4
};

int main(int argc, char *argv[]) {
    parse_arguments(argc, argv);
    if (arguments.date == NULL) {
        fprintf(stderr, "Date parameted is required!\n");
        return DATE_REQUIRED;
    }
    if (arguments.path == NULL) {
        fprintf(stderr, "Path parameted is required!\n");
        return PATH_REQUIRED;
    }
    struct simple_date date_param;
    if (!parse_date(arguments.date, &date_param)) {
        fprintf(stderr, "Invalid date, date must have yyyy-mm-dd format\n");
        return DATE_INVALID;
    }
    // printf("date: %s\n", arguments.date);
	
    struct dirent *item;
    DIR *dir = opendir(arguments.path);
    if (dir) {
        while ((item = readdir(dir)) != NULL) {
            // printf("%s\n", item->d_name);
            // if regular file, not directory
            if (item->d_type == DT_REG) {
                struct simple_date date = date_from_dirent(item);
                if (cmp_dates(&date, &date_param) > 0) {
                    if (arguments.verbose)
                        printf("%d-%d%d-%d%d ", date.y, date.m / 10, date.m % 10, date.d / 10, date.d % 10);
                    printf("%s\n", item->d_name);
                }
            }
        }
        closedir(dir);
    }
    else {
        fprintf(stderr, "Invalid path\n");
        return PATH_INVALID;
    }

    return OK;
}
