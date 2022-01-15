#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

enum rand_errors {
	RND_NO_ERR,
	RND_NO_ARG,
	RND_FOF,	// fopen is failed
	RND_DOF,
	RND_RDEV_F,
	RD_FAILURE,
	WD_FAILURE,
	RND_FOO,
	RND_DOO,
	RND_RDEV_OK,
	RD_OK,
	WD_OK
}; 

struct my_rand {
	FILE *_frnd;
	FILE *_fdata;
	enum rand_errors status;
	uint8_t rand_num;
	int score_stat;
};

void init_rand(struct my_rand *rn, struct arguments *args);

void exit_rand(struct my_rand *rn, struct arguments *args);

void get_num(struct my_rand *rn, struct arguments *args);

void save_data(struct my_rand *rn, struct arguments *args);
