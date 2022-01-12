#include "utils.h"

/* game handler */
static const char *pth_savefile = "data.sav";  // the only save file for now

static enum GAME_ERR __game_load_param(game_t *engine, uint32_t pno, uint32_t pval)
{
    if (NULL == engine)
        return GAME_EARG;
    
    switch (pno)
    {
    case SAV_WINCNT:
        engine->score = pval;
        return GAME_EOK;
        break;
    case SAV_TOTGAM:
        engine->game_count = pval;
        return GAME_EOK;
        break;
    case SAV_BOUNDR:
        engine->boundary = pval;
        return GAME_EOK;
        break;
    case SAV_GUESSV:
        engine->number = pval;
        return GAME_EOK;
        break;
    
    default:
        return GAME_EARG;
        break;
    };
    
    return GAME_EARG;
}

static enum GAME_ERR __game_roll_new(game_t *engine)
{
    if (NULL == engine)
        return GAME_EARG;

    if (engine->flags.is_verbose)
        printf("Rolling new value\n");

    engine->number = rnd_rand(engine->rand_gen, 1, engine->boundary);
    
    return GAME_EOK;
}

static enum GAME_ERR __game_save(game_t *engine)
{
    if (NULL == engine)
        return GAME_EARG;
        
    if (engine->status == GAME_EIO)
        return GAME_EIO;
        
    if (engine->flags.is_verbose)
        printf("Saving game...\n");
        
    FILE *save = fopen(engine->savefile, "wb");
    if (NULL == save) {
        if (engine->flags.is_verbose)
            fprintf(stderr, "Could not open savefile \"%s\"\n", engine->savefile);
        else
            fprintf(stderr, "ERR: %d\n", GAME_EIO);
        return GAME_EIO;
    }
        
    fwrite(&engine->score, sizeof(engine->score), 1, save);
    fwrite(&engine->game_count, sizeof(engine->game_count), 1, save);
    fwrite(&engine->boundary, sizeof(engine->boundary), 1, save);
    fwrite(&engine->number, sizeof(engine->number), 1, save);
        
    fclose(save);
    
    if (engine->flags.is_verbose)
        printf("Game saved\n");
    
    return GAME_EOK;
}

static enum GAME_ERR __game_new(game_t *engine, uint32_t boundary)
{
    if (NULL == engine || boundary <= 1)
        return GAME_EARG;

    engine->score = 0;
    engine->game_count = 0;
    engine->boundary = boundary;
    
    engine->savefile = pth_savefile;

    __game_roll_new(engine);
    
    if (engine->flags.is_verbose)
        printf("Created new save\n");
    
    engine->status = GAME_EOK;
    
    return GAME_EOK;
}

static enum GAME_ERR __game_load(game_t *engine)
{
    if (NULL == engine)
        return GAME_EARG;
        
    engine->score = -1;
    engine->game_count = -1;
    engine->boundary = -1;
    engine->number = -1;
    
    engine->savefile = pth_savefile;
    
    /* save information */
    FILE *save = NULL;
    uint32_t valbuf = -1;
    uint32_t check_it = 0;      // ~ checksum, but not really
        
    /* open save */
    save = fopen(engine->savefile, "rb");
    if (NULL == save) {
        engine->status = GAME_EIO;
        
        if (engine->flags.is_verbose)
            fprintf(stderr, "Save file not found, please start a new game\n");
        else
            fprintf(stderr, "ERR: %d\n", engine->status);
            
        return engine->status;
    }
    
    /* read save information */
    for (uint32_t i = SAV_WINCNT; !feof(save); i++) {
        fread(&valbuf, sizeof(uint32_t), 1, save);       
        __game_load_param(engine, i, valbuf);
        
        check_it = i;
        
        if (check_it > 4)
            break;
    }
    
    if (check_it != 4) {
        engine->status = GAME_ESAV;
        
        if (engine->flags.is_verbose)
            fprintf(stderr, "Corrupted save file, please start a new game\n");
        else
            fprintf(stderr, "ERR: %d\n", engine->status);
    }
    
    fclose(save);
    
    engine->status = GAME_EOK;
    return GAME_EOK;
}

enum GAME_ERR game_init(game_t *engine, uint32_t boundary, enum GAME_STATUS save)
{
    if (NULL == engine)
        return GAME_EARG;
 
    /* init random generator */
    engine->rand_gen = rnd_create();
    if (NULL == engine->rand_gen) {
        
        engine->status = GAME_ERAND;    // unable to allocate rng
        
        if (engine->flags.is_verbose)
            fprintf(stderr, "Failed to initialize RNG\n");
        else
            fprintf(stderr, "ERR: %d\n", engine->status);
        
        return engine->status;
    }
        
    rnd_init(engine->rand_gen);
    
    /* either start anew or load default save */
    if (GAME_NEW == save) { 
        if (__game_new(engine, boundary) != GAME_EOK) {
            engine->status = GAME_EARG;     // boundary is incorrect
            return GAME_EINIT;
        }
    } else if (GAME_LOAD == save) {
        if (__game_load(engine) != GAME_EOK) {
            engine->status = GAME_ESAV;     // save is corrupted
            return GAME_EINIT;
        }
    } else {
        fprintf(stderr, "ERR: %d\n", GAME_EUKW);
    }
    
    __game_save(engine);

    return GAME_EOK;
}

enum GAME_ERR game_free(game_t *engine)
{
    if (NULL == engine)
        return GAME_EARG;
                
    if (NULL != engine->rand_gen)
        free(engine->rand_gen);

    return GAME_EOK;
}

enum GAME_ERR game_play(game_t *engine, uint32_t guess)
{
    if (NULL == engine)
        return GAME_EARG;
        
    if (engine->status == GAME_ESAV) // save was not loaded, data is incorrect
        return GAME_ESAV;            // will break if @status was reset before
        
    if (NULL != engine->rand_gen) {
        engine->game_count++;
        
        if (guess == 0) {
            if (engine->flags.is_verbose)
                printf("It is definetely not zero\n");
            else
                printf("!0\n");
        } else if (guess > engine->number) {
            if (engine->flags.is_verbose)
                printf("The number is smaller than %u\n", guess);
            else
                printf("<%u\n", guess);
        } else if (guess < engine->number) {
            if (engine->flags.is_verbose)
                printf("The number is larger than %u\n", guess);
            else
                printf(">%u\n", guess);
        } else {
            engine->score++;
            
            if (engine->flags.is_verbose) {
                printf("That is correct!\n");
                printf("Won %u of %u games played\n",
                       engine->score, engine->game_count);
            } else {
                printf("+1: %u / %u\n", engine->score, engine->game_count);
            }
            
            __game_roll_new(engine);
        }
    } else {
        engine->status = GAME_ERAND;
        
        if (engine->flags.is_verbose)
            fprintf(stderr, "Failed to initialize RNG\n");
        else
            fprintf(stderr, "ERR: %d\n", engine->status);
            
        return engine->status;
    }
    
    __game_save(engine);

    return GAME_EOK;
}

