#include "my_rand.h"

const char *rnd_err[] = {
	"",
	"No arguments was provided!\n",
	"Cannot open /dev/random!\n",
	"Cannot open database!\n",
	"Cannot read from /dev/random!\n",
	"Cannot read from database!\n",
	"Cannot write to database!\n",
	"\"/dev/random\" was opened successfuly\n",
	"\"database\" was opened successfuly\n",
	"Read from \"/dev/random\" was successful\n",
	"Read from \"database\" was successful\n",
	"Write to \"database\" was successful\n"
};

static void status_check(struct my_rand *rn, struct arguments *args)
{
	fprintf(rn->status >= 7 ? stdout : stderr, "%s%s%s",
		rn->status >= 7 ? color(BLUE) : color(RED),
		args->verbose && rn->status != 0 ? (rn->status >= 7 ? "Rand: " : "Error: ") : "",
		(rn->status < 7 || args->verbose) ? rnd_err[rn->status] : "");
	if(rn->status < 7 && rn->status > 0) {
		fclose(rn->_frnd);
		rn->_frnd = NULL;
		fclose(rn->_fdata);
		rn->_fdata = NULL;
		exit(rn->status);
		}
	return;
}

void init_rand(struct my_rand *rn, struct arguments *args)
{
	rn->status = RND_NO_ERR;
	if (NULL == rn)
		rn->status = RND_NO_ARG;
	
	status_check(rn, args);
	
	rn->score_stat = 0;
	rn->_frnd = fopen("/dev/random", "rb");
	rn->_fdata = fopen("database.rst", "r+");
	
	if (NULL == rn->_frnd) rn->status = RND_FOF;
	else rn->status = RND_FOO;

	status_check(rn, args);
	
	if (NULL == rn->_fdata) rn->status = RND_DOF;
	else rn->status = RND_DOO;

	status_check(rn, args);
	return;
}

void exit_rand(struct my_rand *rn, struct arguments *args)
{
	if (NULL == rn) {
		rn->status = RND_NO_ARG;
		goto exit_end;
	}
	fclose(rn->_frnd);
	rn->_frnd = NULL;
	fclose(rn->_fdata);
	rn->_fdata = NULL;
	rn->status = RND_NO_ERR;
exit_end:
	status_check(rn, args);
	exit(0);
	return;
}

void get_num(struct my_rand *rn, struct arguments *args)
{
	size_t read_stat = 1;
	rn->status = RND_NO_ERR;
	if (NULL == rn) 
		rn->status = RND_NO_ARG;
	
	status_check(rn, args);
	
	read_stat = fread(&(rn->rand_num), sizeof(rn->rand_num), 1, rn->_frnd);
	
	if(!read_stat) rn->status = RND_RDEV_F;
	else rn->status = RND_RDEV_OK;
	
	status_check(rn, args);
	
	if(args->verbose)
		fprintf(stdout, "\tRandom byte is %u\n", rn->rand_num);

	rn->rand_num %= 6;
	rn->rand_num++;

	return;
}

void save_data(struct my_rand *rn, struct arguments *args)
{
	uint8_t scores[3] = {0, 0, 0};
	size_t rw_stat;
	rn->status = RND_NO_ERR;
	if (NULL == rn)
		rn->status = RND_NO_ARG;
	
	status_check(rn, args);

	fseek(rn->_fdata, 0, SEEK_SET);

	if(!(args->new_game)) {
		rw_stat = fread(scores, sizeof(*scores), 3, rn->_fdata);
		if(scores[2] == 0) rw_stat = 3;
	}
	
	if(rw_stat < 3)	rn->status = RD_FAILURE;
	else rn->status = RD_OK;

	status_check(rn, args);

	if(args->verbose) {
		fprintf(stdout, "\tCurrent values are: %u, %u, %u\n",
				scores[0], scores[1], scores[2]);
	}

	rn->score_stat ? scores[1]++ : scores[0]++;
	scores[2]++;	// this is the number of played games
	
	fseek(rn->_fdata, 0, SEEK_SET);
	rw_stat = fwrite(scores, sizeof(*scores), 3, rn->_fdata);
	
	if(rw_stat < 3) rn->status = WD_FAILURE;
	else rn->status = WD_OK;

	status_check(rn, args);

	if(args->verbose) {
		fprintf(stdout, "\tNew values are: %u, %u, %u\n",
				scores[0], scores[1], scores[2]);
	}

	fprintf(stdout, "%sComputer scores:\tYour scores:\n%u\t\t\t%u\n"
			"Games played: %d\n",
			color(WHITE),
			scores[0], scores[1], scores[2]);

	return;
}
