#include "rand.h"

int game(void)
{
	int player_score = -1;
	int computer_score = -1;

	// Data for pseudo animations
	int player_sequence[PS_ANIM_SEQ_LENGTH];
	int computer_sequence[PS_ANIM_SEQ_LENGTH];

	init_sequence(player_sequence, PS_ANIM_SEQ_LENGTH);
	init_sequence(computer_sequence, PS_ANIM_SEQ_LENGTH);

	player_score = player_sequence[PS_ANIM_SEQ_LENGTH - 1];
	computer_score = computer_sequence[PS_ANIM_SEQ_LENGTH - 1];

	pseudo_animation("Human", player_sequence, PS_ANIM_SEQ_LENGTH);
	pseudo_animation("Computer", computer_sequence, PS_ANIM_SEQ_LENGTH);

	if (player_score == -1 || computer_score == -1) {
		return GAME_ERROR;
	}

	if (player_score > computer_score) {
		printf("Human win!\n");
	} else if (player_score < computer_score) {
		printf("Computer win\n");
	} else {
		printf("Draw!\n");
	}

	return GAME_OK;
}

void init_sequence(int seq[], int seq_length)
{
	for (int i = 0; i < seq_length; i++) {
		seq[i] = rand() % MAX_RND;
	}
}

int delay(long milli_seconds)
{
	struct timespec ts;
	int res;

	if (milli_seconds < 0) {
		return -1;
	}

	ts.tv_sec = milli_seconds / 1000;
	ts.tv_nsec = (milli_seconds % 1000) * 1000000;

	do {
		res = nanosleep(&ts, &ts);
	} while (res);

	return res;
}

void pseudo_animation(const char *message, int seq[], int seq_length)
{
	printf("%s:\n\n", message);

	const int upper_delay_ms = 250;
	const int lower_delay_ms = 125;
	for (int i = 0; i < seq_length - 1; i++) {
		char num_ascii = seq[i] + '0';
		// Get up on one line \033[A and remove all from it \33[2K
		printf("\033[A\33[2K%c\n", num_ascii);

		int delay_ms =
			(rand() % (upper_delay_ms - lower_delay_ms + 1)) +
			lower_delay_ms;
		delay(delay_ms);
	}

	printf("\033[A\33[2K%c\n", (seq[seq_length - 1] + '0'));
}
