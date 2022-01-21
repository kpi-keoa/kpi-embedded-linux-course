#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

enum Verbose {
    VERBOSE_DISABLE,
    VERBOSE_ENABLE
};

enum Dist {
    DIST_DISABLE,
    DIST_ENABLE
};

enum Errors {
    EXECUTION_SUCCESSFUL,
    NO_ARGUMENTS,
    INVALID_POINT,
    NOT_ENOUGH_DATA,
    MEMORY_ERROR,
    INVALID_POINT_SET
};

typedef struct tagPoint {
	double x;
	double y;
} Point;

typedef struct tagArray {
    int number; // amount of all points
    int counter; // current amount of points
    double distance;

    Point** points;
} Array;

Point *create_point(Point *aPoint);
Array *create_array(int aNumber);
void add_element(Array *anArray, Point *aPoint);
void view_help(void);
void destroy_point(Point *aPoint);
void print_point(Point *aPoint, int i);
void print_array(Array *anArray);
void free_array(Array *anArray);
void equidistant_point(Array *anArray);
