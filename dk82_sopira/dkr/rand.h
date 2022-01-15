#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* random module */
enum RND_ERR {
    RND_EOK = -0,
    RND_EARG = -1,
    RND_EIO = -2
};

typedef struct rnd_s {
    FILE *_file;
} rnd_t;

rnd_t *rnd_create(void);
enum RND_ERR rnd_init(rnd_t *rnd);

enum RND_ERR rnd_free(rnd_t *rnd);

uint32_t rnd_get32(rnd_t *sys_rand);

uint32_t rnd_rand(rnd_t *sys_rand, uint32_t min, uint32_t max);
