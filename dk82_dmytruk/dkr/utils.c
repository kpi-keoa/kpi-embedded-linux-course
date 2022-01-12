#include "utils.h"

extern time_dir check_files_inst;



int check_file_time(time_dir* condition, struct tm* check_file)
{
    if((condition->day) < (check_file->tm_mday)){
        return 1; // good
    }else if ((condition->day) > (check_file->tm_mday)){
        return 0; // bad
    }else{
        if((condition->hour) < (check_file->tm_hour)){
            return 1; // good
        }else if ((condition->hour) > (check_file->tm_hour)){
            return 0; // bad
        }else{
            if((condition->min) <= (check_file->tm_min)){
                return 1; // good
            }else {
                return 0; // bad
            }
        }
    }
}

void copy_time_to_inst (time_dir* check_file, int day, int hour, int min)
{
    check_file->day = day;
    check_file->hour = hour;
    check_file->min = min;
}

void ls(const char *dir, enum Verbose arg_verb)
{

    struct stat t_stat;

	struct dirent *d;
	DIR *dh = opendir(dir);

	if(!dh){
		if(errno = ENOENT){
			//If the directory is not found
            //fprintf(stderr, "%s", "Directory is not found!\n");
            fprintf(stderr,"Directory %s is not found!\n", dir);
            exit(EXIT_FAILURE);
		}else if(arg_verb == VERBOSE_ENABLE){
            fprintf(stdout, "%s", "Unable to read directory\n");
		}
	}

	while((d = readdir(dh)) != NULL){
        stat(d->d_name, &t_stat);
        struct tm * timeinfo = localtime(&t_stat.st_ctime);

        if(check_file_time (&check_files_inst, timeinfo)){
            if(arg_verb == VERBOSE_DISABLE){
                printf("%s\n",d->d_name);
            }else{
                printf("File: %s, mday: %i, hours: %i, min: %i\n",d->d_name,
                    timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min);
            }
        }
	}
	printf("\n");
}

enum Errors check_date_valid (time_dir* inst){
    if(inst->day > 31 ||  inst->hour > 24 || inst->min > 60)
        return DATE_ERROR; // BAD
    else
        return 0; // GOOD
}
