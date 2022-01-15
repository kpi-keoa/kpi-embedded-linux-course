#include "midp.h"


enum RUN_STATUS status = _OK;

static struct argp_option options[] = {
        {"dist", 'd', "DISTANCE", OPTION_ARG_OPTIONAL, "Show distance to points"},
        {"verbose", 'v', "VERBOSITY", OPTION_ARG_OPTIONAL, "Produce verbose outupt"},
        {0,0,"COORDINATES",0, "Coordinates X Y"}
};

static char doc[] = "Find middle point between points";       // Program documentation

static char args_doc[] = "Input X and Y coordinates of points to find middle point.\nTo input negative number input -- before coordinates";   // Usage

static struct argp parse_args = { options, parse_opt, args_doc, doc };


enum COOR_ARR_STATUS arr_status_main;
enum RUN_STATUS run_status;
enum CIRCLE_STATUS circle_status;
enum PointStatus point_status;

static int verbose;
static int dist;

int main(int argc, char **argv){

        circle_status = _CIRCLE_OK;
        point_status = _ON_CIRCLE;

        struct Parse_Args_Global args;
        args.verbose = 0;
        args.dist = 0;
        args.size = 0;

        argp_parse(&parse_args, argc, argv, 0, 0, &args);

        verbose = args.verbose;
        dist = args.dist;

        if(verbose){
                print_Parse_Args_Global(&args);
        }

        arr_status_main = checkSizeArrayOfCoordinates(args.size);
        run_status = checkError();

        args.size = checkCoordinatesInArray(args.coordinates, args.size);

        if(verbose){
                print_Parse_Args_Global(&args);
        }

        arr_status_main = checkSizeArrayOfCoordinates(args.size);
        run_status = checkError();

        struct MiddlePoint midPoint;

        if(args.size == 4){
                midPoint = twoPoints(args.coordinates);


        }else{
                midPoint = calcCircle(args.coordinates, args.size);
                if(args.size == 3){
                    point_status = _ON_CIRCLE;
                }
                if(midPoint.length == -1){
                        circle_status = _CIRCLE_TRIANGLE_DEGENERATE;
                }
        }

        checkError();

        if(args.size > (3 * 2)){
            point_status = checkDotsOnCircle(args.coordinates, args.size, &midPoint);
        }

        checkError();

        printDataMidPoint(&midPoint);

        return run_status;
}


int checkSizeArrayOfCoordinates(int arr_len){
        enum COOR_ARR_STATUS arr_status = _OK;

        if(arr_len <= 3){
                arr_status = _ERR_TOO_SMALL;
        }else if(arr_len % 2 != 0){
                fprintf(stderr, "Odd\n");
                arr_status = _ERR_ODD_SIZE;
        }

        return arr_status;
}


int checkCoordinatesInArray(int *arr, int arr_len){
        /*
         * This function is called after length check and atoi check,
         * so we sure there is no error with coordinates numbers
         */
        int x, y, check_x, check_y;
        int points = arr_len / 2;

        int i;
        int j;

        int swap_to_prev = 0;

        for (i = 0; i < arr_len - 1; i += 2){
                if(swap_to_prev){
                        points--;
                        arr_len -= 2;
                        i -= 2;
                        swap_to_prev = 0;
                }

                x = arr[i];
                y = arr[i+1];

                for (j = i+2; j < arr_len; j += 2){
                        check_x = arr[j];
                        check_y = arr[j+1];

                        if(swap_to_prev){
                                arr[j-2] = arr[j];      // x
                                arr[j-1] = arr[j+1];    // y

                                arr[j] = 0;
                                arr[j+1] = 0;

                        }

                        if(x == check_x && y == check_y){
                                // if found duplicate point then delete it and shift other points to left
                                swap_to_prev = 1;
                        }

                          if(verbose){
                                fprintf(stdout, "points = %i\nxi = %i, yi = %i\nxj = %i, yj = %i, swap_to_prev = %i\n",
                                        points,x, y, check_x, check_y, swap_to_prev);
                        }

                }
        }

        return arr_len;
}

struct MiddlePoint twoPoints(int *arr){
        struct MiddlePoint coordinates;

        double x = (arr[0] + arr[2]) / 2.0;
        double y = (arr[1] + arr[3]) / 2.0;
        double length = sqrt(((arr[0] - arr[2]) * (arr[0] - arr[2])) + ((arr[1] - arr[3]) * (arr[1] - arr[3])));

        coordinates.x = x;
        coordinates.y = y;
        coordinates.length = length / 2;

        return coordinates;
}

struct MiddlePoint calcCircle(int *arr, int arr_len){

        struct MiddlePoint coordinates;
        double x, y, length;



        // from circle equation
        double a, b, c, d, e, f, g;
        int i = 0;
        int on_same_plane = 1;



        a = arr[2] - arr[0]; // x1 - x0
        b = arr[3] - arr[1]; // y1 - y0

        e = a * (arr[2] + arr[0]) + b * (arr[3] + arr[1]);

        while(on_same_plane){          // iterate throw all dots if g == 0 to find non 0
                c = arr[4 + i * 2] - arr[0]; // x2 - x0, then xn(n >= 2) - x0, if g == 0
                d = arr[5 + i * 2] - arr[1]; // y2 - y0, then yn(n >= 2) - y0, if g == 0

                f = c * (arr[4 + i * 2] + arr[0]) + d * (arr[5 + i * 2] + arr[1]);

                g = 2 * (a * (arr[5 + i * 2] - arr[3])) - b * (arr[4 + i * 2] - arr[2]);

                if(g == 0){
                        on_same_plane = 1;
                        i++;
                        if(i > ((arr_len / 2) - 3)){
                                coordinates.x = 0;
                                coordinates.y = 0;
                                coordinates.length = -1;        // error
                                return coordinates;
                        }
                }else{
                        on_same_plane = 0;
                        x = (d * e - b * f) / g;
                        y = (a * f - c * e) / g;

                        length = sqrt(((arr[0] - x)*(arr[0] - x))+((arr[1] - y)*(arr[1] - y)));

                        break;
                }
        }

        coordinates.x = x;
        coordinates.y = y;
        coordinates.length = length;

        return coordinates;
}



int checkDotsOnCircle(int *arr, int arr_len, struct MiddlePoint *midPoint){
        int i;
        double length;

        enum PointStatus pointStat = _ON_CIRCLE;

        for(i = 0; i < arr_len; i += 2){
                length = sqrt(((arr[i] - midPoint->x)*(arr[i] - midPoint->x))+((arr[i + 1] - midPoint->y)*(arr[i + 1] - midPoint->x)));

                if(verbose){
                        fprintf(stdout,"Point[%i] x = %i, y = %i\nCenter: x = %f, y = %f\nlength calc = %f, length circle = %f\n",
                                        (i / 2), arr[i], arr[i + 1], midPoint->x, midPoint->y, length, midPoint-> length);
                }

                if(length != midPoint->length){
                        return _NOT_ON_CIRCLE;
                }
        }

        return pointStat;
}


void printDataMidPoint(struct MiddlePoint *midPoint){

        fprintf(stdout,"Middle Point: X = %f, Y = %f\n",midPoint->x, midPoint->y);

        if(dist){
                fprintf(stdout,"Middle Point: distance = %f\n",midPoint->length);
        }
}

enum RUN_STATUS checkError(){
                enum RUN_STATUS status = _OK;

                if(arr_status_main){
                        fprintf(stderr, "There should be %s",
                                (arr_status_main == _ERR_ODD_SIZE) ? "even number of coordinates!\n" : "more different coordinates!\n");
                        status = _ERR;
                }

                if(circle_status){
                        fprintf(stderr, "Circumscribed triangle is degenerate. Dot`s shouldn`t be on one line!\n");
                        status = _ERR;
                }

                if(point_status){
                        fprintf(stderr, "One or more point don`t form cyclic polygon, no equidistant point for such set of points!\n");
                        status = _ERR;
                }

                if(status){
                        exit(status);
                }

                return _OK;
}
