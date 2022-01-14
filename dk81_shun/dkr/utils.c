#include "utils.h"

void view_help() {
    fprintf(stdout, "________________________________________________________________________\n");
    fprintf(stdout, "This program compare two files by their size\n");
    fprintf(stdout, "------------------         ------------------         ------------------\n");
    fprintf(stdout, "Arguments:\n");
    fprintf(stdout, "--verbose (-v) - Show more information during program perfomance\n");
    fprintf(stdout, "--trim - Cut the contents of bigger file to the size of smaller\n");
    fprintf(stdout, "--help - View help\n");
    fprintf(stdout, "------------------         ------------------         ------------------\n");
    fprintf(stdout, "Author: Pasha Shun || tg: @world_conspiracy || Linkedin: worldconspiracy\n");
    fprintf(stdout, "________________________________________________________________________\n");
}
