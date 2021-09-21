/* random module */

static const char pth_random[] = "/dev/random";
static const char pth_urandom[] = "/dev/urandom";

#include "rand.h"

enum RND_ERR rnd_init(rnd_t *rnd, enum RND_TYPE type)
{
    if (NULL == rnd)
        return RND_EARG;

    rnd->type = type;

    rnd->_file = fopen((RND_TYPE_RANDOM == rnd->type) ? pth_random : pth_urandom, "rb");

    if (NULL == rnd->_file)
        return RND_EIO;

    return RND_EOK;
}

enum RND_ERR rnd_free(rnd_t *rnd, enum RND_TYPE type)
{
    if (NULL == rnd || NULL == rnd->_file)
        return RND_EARG;

    fclose(rnd->_file);
    rnd->_file = NULL;

    return RND_EOK;
}

size_t rnd_get(rnd_t *rnd, uint8_t *arr, size_t len)
{
    if (NULL == rnd || NULL == rnd->_file)
        return RND_EARG;

    return fread(arr, sizeof(*arr), len, rnd->_file);
}

uint32_t srnd_get32(void)
{
    if (NULL == SYS_RAND_PTR || NULL == SYS_RAND_PTR->_file)
        return RND_EARG;

    uint32_t result = -1;
    fread(&result, sizeof(uint32_t), 1, SYS_RAND_PTR->_file);

    return result;
}
