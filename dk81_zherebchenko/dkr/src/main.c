#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <math.h>
#include <argp.h>       // Interface for parsing unix-style argument vectors
#include "utils.h"      // Contains part of the service functions

static float equdist_point[2];          // Coordinates of the equidistant point

static float* equidist_point_twop(float *x, float *y);
static float* equidist_point_threep(float *x, float *y);
static void check_triangle(float *x, float *y, int size);
static void check_points(float *x, float *y, int size, float dist);
static void print_results(int verbose_flag, int dist_flag, float dist);

static char doc[] = "The program outputs the coordinates of the equidistant point from "
                        "the others, as well as the distance to it";
static char args_doc[] = "Accepts an arbitrary number of X Y coordinates for points, and "
                                "the --dist flag for distance";

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char *argv[])
{
        float *x, *y;                   // For save coordinates
        int i, j;
        float d;                        // Distance to the equidistant point
        int size;                       // Number of coordinates

        struct arguments arguments;
        
        arguments.verbose = 0;
        arguments.dist = 0;       
        arguments.args = malloc(argc * sizeof(*arguments.args));
        
        argp_parse (&argp, argc, argv, 0, 0, &arguments);
        
        if (arguments.verbose && arguments.dist) {
                size = 0.5 * (argc - 3);
        } else {
                size = 0.5 * (argc - 1);
        }
        
        x = (float*)malloc(size * sizeof(float));
        y = (float*)malloc(size * sizeof(float));
        
        j = 0;
        for (i = 0; i < size; i++) {
                x[i] = atoi(arguments.args[j]);
                y[i] = atoi(arguments.args[j + 1]);
                j = j + 2;
        }

        check_triangle(x, y, size);                     // Checking the existence of all triangles

        if (size == 2) {
                equidist_point_twop(x, y);              // Calculation for two points
        } else {
                equidist_point_threep(x, y);            // Calculation for three or more points
        }
        
        d = sqrt(pow((equdist_point[1] - x[0]), 2) + pow((equdist_point[2] - y[0]), 2));

        check_points(x, y, size, d);                    // Checking other points for equidistance
        
        print_results(arguments.verbose, arguments.dist, d);

        free(x);
        free(y);
        free(arguments.args);

        exit(0);
}

static float* equidist_point_twop(float *x, float *y)
{
        int i;
        for (i = 0; i < 2; i++) {
                equdist_point[1] = equdist_point[1] + x[i] / 2;
                equdist_point[2] = equdist_point[2] + y[i] / 2;
        }
        
        return equdist_point;
}

static float* equidist_point_threep(float *x, float *y)
{
        equdist_point[1] = -((y[0] - y[1]) * (pow(x[2], 2) + pow(y[2], 2)) 
                + (y[1] - y[2]) * (pow(x[0], 2) + pow(y[0], 2)) 
                + (y[2] - y[0]) * (pow(x[1], 2) + pow(y[1], 2))) 
                / (2 * ((x[0] - x[1]) * (y[2] - y[0]) - (y[0] - y[1]) * (x[2] - x[0])));
        equdist_point[2] = ((x[0] - x[1]) * (pow(x[2], 2) + pow(y[2], 2)) 
                + (x[1] - x[2]) * (pow(x[0], 2) + pow(y[0], 2)) 
                + (x[2] - x[0]) * (pow(x[1], 2) + pow(y[1], 2))) 
                / (2 * ((x[0] - x[1]) * (y[2] - y[0]) - (y[0] - y[1]) * (x[2] - x[0])));

        return equdist_point;
}

static void check_triangle(float *x, float *y, int size)
{
        int i, j, l;
        float k, b;

        for (l = 0; l < size; l++) {
                for (i = 0; i < size; i++) {
                        for (j = 0; j < size; j++) {
                                // y = k * x + b
                                k = (y[i] - y[j]) / (x[i] - x[j]);
                                b = y[j] - k * x[j];
                                if ((y[l] == k * x[l] + b) && (l != i) && (l != j)) {
                                        fprintf(stderr, "There are points that coincide, "
                                                "or are on the same line\n");
                                        exit(0);
                                }
                        }
                }
        }
}

static void check_points(float *x, float *y, int size, float dist)
{
        int i;
        
        for (i = 3; i < size; i++) {
                if ((float)sqrt(pow((equdist_point[1] - x[i]), 2) 
                        + pow((equdist_point[2] - y[i]), 2)) != dist) {
                        fprintf(stderr, "Equidistant point doesn't exist\n");
                        exit(0);
                }
        }
}

static void print_results(int verbose_flag, int dist_flag, float dist)
{
        if (!verbose_flag) {
                fprintf(stdout, "%f %f\n", equdist_point[1], equdist_point[2]);
                if (dist_flag) {
                        fprintf(stdout, "%f", dist);
                }
        } else {
                fprintf(stdout, "Coordinates of the equidistant point: x = %f, y = %f\n", 
                        equdist_point[1], equdist_point[2]);
                if (dist_flag) {
                        fprintf(stdout, "Distance to the equidistant point: %f", dist);
                }
        }
}

