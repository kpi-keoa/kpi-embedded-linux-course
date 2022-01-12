#include "rand.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

/* verbose and such */
enum VERB_FLAG {
    VERBOSE_ENABLE = 0,
    VERBOSE_DISABLE = 1
};

/* game handler */
enum GAME_STATUS {
    GAME_NEW,
    GAME_LOAD
};

typedef struct glob_flags {
    uint8_t is_verbose : 1;
    uint8_t is_newgame : 1;
    uint8_t is_loadgame : 1;
} gflags_t;

enum SAV_PARAM {
    SAV_WINCNT = 0,
    SAV_TOTGAM = 1,
    SAV_BOUNDR = 2,
    SAV_GUESSV = 3
};

enum GAME_ERR {
    /* general errors */
    GAME_EOK = 0,
    GAME_EINIT = -1,
    GAME_EIO = -2,
    GAME_EARG = -3,
    GAME_ERAND = -4,
    /* spesific errors */
    GAME_ESAV = -5,
    GAME_EUKW = -255
};

typedef struct game_struct {
    rnd_t *rand_gen;
    gflags_t flags;
    const char *savefile;
    
    /* used for specific signals */
    uint32_t status;
        
    uint32_t score;
    uint32_t game_count;
    
    uint32_t number;
    uint32_t boundary;
} game_t;

enum GAME_ERR game_init(game_t *engine, uint32_t boundary, enum GAME_STATUS save);
enum GAME_ERR game_play(game_t *engine, uint32_t guess);
enum GAME_ERR game_free(game_t *engine);
