#include "utils.h"

error_t parse_opt(int key, char *arg, struct argp_state *state)
{
        enum PARSE_STATUS parse_stat = _PARSE_OK;
        enum ATOI_STATUS atoi_status = _ATOI_OK;

        struct Parse_Args_Global *arguments = state->input;      // get struct as parameter via *state
        switch(key)
        {
                case 'v':
                        arguments->verbose = 1;
                        break;
                case 'd':
                        arguments->dist = 1;
                        break;
                case ARGP_KEY_ARG:
                        if(arguments->verbose ==1){
                                fprintf(stdout,"arguments = %s\n",arg);
                        }
                        atoi_status = addNewCoordinate(arguments,arg);

                        if(atoi_status){
                               parse_stat =  _PARSE_ERR;
                               fprintf(stderr, "Coordinate arguments should be integers!\n");
                               return ARGP_KEY_ERROR;
                        }

                        break;
                case ARGP_KEY_END:
                        if (state->arg_num < 1){
                                argp_failure (state, 1, 0, "Too few arguments");
                                argp_usage (state);
                        }
                        break;
                default:
                        return ARGP_ERR_UNKNOWN;
        }

       return parse_stat;
}

int addNewCoordinate(struct Parse_Args_Global *args, char *coordinate){

         enum ATOI_STATUS atoi_status = _ATOI_OK;

         int coordinate_int = atoi(coordinate);
         int coordinate_strlen = strlen(coordinate);

         if((coordinate_int == 0 && coordinate_strlen > 1) || ((coordinate_int == 0) && (coordinate_strlen == 1) && (coordinate[0] != '0'))){
                atoi_status = _ATOI_ERR;
                return atoi_status;
         }

         if(args->size == 0){
                args->coordinates[0] = coordinate_int;
         }else{
                args->coordinates[args->size] = coordinate_int;
         }
         args->size = args->size + 1;

         return atoi_status;
}

void print_Parse_Args_Global(struct Parse_Args_Global *args){
        fprintf(stdout,"Dist = %i\nVerbose = %i\nSize = %i\n",
                        args->dist, args->verbose, args->size);
        int i;
        for(i = 0; i < args->size; i++){
                 fprintf(stdout,"args->coordinates[%i] = %i\n",i, args->coordinates[i]);
        }
}
