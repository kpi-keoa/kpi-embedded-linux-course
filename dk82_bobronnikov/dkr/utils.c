#include "utils.h"

__uint8_t generate_num(void)
{
	__uint8_t num;
	fp = fopen("/dev/urandom", "r");
	fread(&num, sizeof(num), 1, fp);
	fclose(fp);
	num = num % 16;
	return num;
}


void read_stats(int *score, int *count)
{
	fp = fopen("./stats.txt", "r");
	if (fp == NULL){
		*score = 0; 
		*count = 0;
	}
	else {
		fscanf(fp, "%d %d", score, count);
		fclose(fp);
	}
}

void write_scores(int score, int count)
{
	fprintf(stdout, "%d : %d\n", score, count);
	fp = fopen("./stats.txt", "w+");
	fprintf(fp, "%d %d", score, count);
	fclose(fp);
}
