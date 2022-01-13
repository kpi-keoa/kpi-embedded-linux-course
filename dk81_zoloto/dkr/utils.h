#pragma once

#include <time.h>   // time_t
#include <dirent.h> // dirent

//struct simple_date;
struct simple_date {
    int y, m, d;
};

//typedef struct simple_date tdate;

int cmp_dates(struct simple_date *d1, struct simple_date *d2);
int parse_date(const char *s, struct simple_date *d);
struct simple_date get_date(time_t t);
struct simple_date date_from_dirent(struct dirent *item);
