//struct simple_date {
//    int y, m, d;
//};

#include "utils.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> 

//typedef struct simple_date struct simple_date;

int cmp_dates(struct simple_date *d1, struct simple_date *d2) {
    int y = d1->y - d2->y;
    int m = d1->m - d2->m;
    int d = d1->d - d2->d;

    if (y != 0) return y;
    if (m != 0) return m;
    if (d != 0) return d;
    return 0;
}

int parse_date(const char *s, struct simple_date *d) {
    return sscanf(s, "%d-%d-%d", &(d->y), &(d->m), &(d->d)) == 3;
}

struct simple_date get_date(time_t t) {
    struct tm *tm = localtime(&t);
    struct simple_date d;
    d.d = tm->tm_mday;
    d.m = tm->tm_mon + 1;
    d.y = tm->tm_year + 1900;
    // printf("%d %d %d\n", d->d, d->m, d->y); // debug only
    return d;
}

struct simple_date date_from_dirent(struct dirent *item) {
    struct stat finf;
    stat(item->d_name, &finf);
    time_t tm = finf.st_mtime;
    // printf("%ld\n", tm); // debug only
    struct simple_date res = get_date(tm);
    return res;
}
