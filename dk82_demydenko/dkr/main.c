#include "utils.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <argp.h>

struct arguments {
    point_t **parr;
    uint32_t points;
};

static bool verbose = false;
 
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *args = state->input;
        
    switch (key)
    {        
    case 'v':
        verbose = true;
        break;
        
    case 'p':        
        if (NULL == arg) {
            argp_usage(state);
            break;
        }      
        
        /* split string by ',' and get coords */
        char *buf = NULL;
        double x = 0.f, y = 0.f;
        uint8_t p = 0;  // ~ parity flag
        
        buf = strtok(arg, ",");
        while(buf != NULL) {
            if (p == 0) {
                x = atof(buf);
            } else {
                y = atof(buf);
               
                args->parr[args->points] = point_create('P', x, y);
                args->points++;
            }
                
            p ^= 1;
            
            buf = strtok(NULL, ",");
        }
        break;

    case ARGP_KEY_END:
        if (state->argc > 4 || state->argc <= 2)
            argp_usage(state);
        break;
        
    default:
        return ARGP_ERR_UNKNOWN;
        break;
    };
    
    return 0;
}

int main(int argc, char **argv)
{
    /* options */
    struct argp_option options[] = {
        { "verbose", 'v', 0, 0, "Produce verbose output" },
        { "point", 'p', "[x,y]", 0, "A point" },
        { 0 }
    };
    struct argp argp = { options, parse_opt, 0, 0 };
    
    struct arguments in_arg = { NULL, 0 };
    
    in_arg.parr = malloc(sizeof(in_arg.parr));
    if (NULL == in_arg.parr) {
        fprintf(stderr, "No memory left\n");
        return ERR_MEM;
    }
    
    argp_parse(&argp, argc, argv, 0, 0, &in_arg);
    
    /* main */   
    if (in_arg.points == 2) {
        if (NULL == in_arg.parr[0] || NULL == in_arg.parr[1] )
            return ERR_MEM;
        
        point_t *A = in_arg.parr[0];
        point_t *B = in_arg.parr[1];
        
        point_t center;
        center.n = 'O';
        center.x = (A->x + B->x) / 2;
        center.y = (A->y + B->y) / 2;
        
        if (verbose)
            printf("Center point is:\n");
        
        point_print(&center);
        
    } else if (in_arg.points == 3) {
        triangle_t *triangle =
        triangle_create(in_arg.parr[0], in_arg.parr[1], in_arg.parr[2]);
        
        if (NULL == triangle) {
            fprintf(stderr, "No memory left\n");
            return ERR_MEM;
        }
            
        point_t *center = triangle_center(triangle);
        if (NULL == center) {
            fprintf(stderr, "No memory left\n");
            return ERR_MEM;
        }
        
        if (verbose)
            printf("Center point is:\n");
            
        point_print(center);
        
        point_free(center);
        triangle_free(triangle);
    } else {
        fprintf(stderr, "Invalid amount of points: %u\n", in_arg.points);
    }
    
    if (NULL != in_arg.parr) {
        for (uint32_t i = 0; i < in_arg.points; i++)
            point_free(in_arg.parr[i]);
        
        free(in_arg.parr);
    }
        
    return ERR_OK;
}
