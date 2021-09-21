#include <stdio.h>
#include <stdint.h>

/* random module */
enum RND_ERR { RND_EOK = 0, RND_EARG = 1, RND_EIO = 2 };

enum RND_TYPE { RND_TYPE_RANDOM = 0, RND_TYPE_URANDOM = 1 };

typedef struct rnd_s {
    enum RND_TYPE type;
    FILE *_file;
} rnd_t;

static rnd_t SYS_RAND;
static rnd_t *SYS_RAND_PTR = &SYS_RAND;

enum RND_ERR rnd_init(rnd_t *rnd, enum RND_TYPE type);

enum RND_ERR rnd_free(rnd_t *rnd, enum RND_TYPE type);

size_t rnd_get(rnd_t *rnd, uint8_t *arr, size_t len);

uint32_t srnd_get32(void);
