#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#if defined(VOLATILE) && VOLATILE
	volatile unsigned long count;
#else
	unsigned long count;
#endif

typedef unsigned long count_type;

struct the_val {
    count_type val;
};

void *increment(void *val_str)
{
    count_type value = ((struct the_val *)val_str)->val;

    for (int i = 0; i < value; i++)
        count++;
}

int main(int argc, char *argv[])
{
    pthread_t thread_id;

    if (argc != 2) {
        fprintf(stderr, "%s : wrong ammount of argumens : %d ; should be 1 \n",
			argv[0], argc - 1);
        exit(1);
    }

    count_type value = strtoul(argv[1], NULL, 10);

    struct the_val pth_val = { .val = value };

    pthread_create(&thread_id, NULL, increment, &pth_val);

    for (int i = 0; i < value; i++)
        count++;

    pthread_join(thread_id, NULL);

    printf("expected = %ld; the resault = %ld \n", 2 * value, count);

    return 0;
}
