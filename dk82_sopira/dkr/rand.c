/* random module */

static const char *pth_random = "/dev/random";

#include "rand.h"

rnd_t *rnd_create(void)
{
    rnd_t *rnd = NULL;
    rnd = malloc(sizeof(*rnd));

    return rnd;
}

enum RND_ERR rnd_init(rnd_t *rnd)
{
    if (NULL == rnd)
        return RND_EARG;

    rnd->_file = fopen(pth_random, "rb");

    if (NULL == rnd->_file)
        return RND_EIO;

    return RND_EOK;
}

enum RND_ERR rnd_free(rnd_t *rnd)
{
    if (NULL == rnd)
        return RND_EARG;

    fclose(rnd->_file);
    free(rnd);

    return RND_EOK;
}

uint32_t rnd_get32(rnd_t *sys_rand)
{
    if (NULL == sys_rand || NULL == sys_rand->_file)
        return RND_EARG;

    uint32_t result = -1;
    fread(&result, sizeof(uint32_t), 1, sys_rand->_file);

    return result;
}

uint32_t rnd_rand(rnd_t *sys_rand, uint32_t min, uint32_t max)
{
    if (NULL == sys_rand || NULL == sys_rand->_file)
        return RND_EARG;

    return (min + rnd_get32(sys_rand) % max);
}
