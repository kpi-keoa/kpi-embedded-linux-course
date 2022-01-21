#include <stdio.h>
#include "utils.h"
#include <stdlib.h>
#include <math.h>

enum RUN_STATUS
{
        _OK = 0,
        _ERR = 1
} run_stat;


enum COOR_ARR_STATUS
{
        _ARR_OK = 0,
        _ERR_ODD_SIZE = 1,
        _ERR_TOO_SMALL = 2
};

enum CIRCLE_STATUS
{
        _CIRCLE_OK = 0,
        _CIRCLE_TRIANGLE_DEGENERATE = 1
};


enum PointStatus{
        _ON_CIRCLE = 0,
        _NOT_ON_CIRCLE = 1
};

struct MiddlePoint
{
        double x, y, length;
};


int main(int argc, char **argv);

int checkSizeArrayOfCoordinates(int arr_len);

int checkCoordinatesInArray(int *arr, int arr_len);

struct MiddlePoint twoPoints(int *arr);

struct MiddlePoint calcCircle(int *arr, int arr_len);

int checkDotsOnCircle(int *arr, int arr_len, struct MiddlePoint *midPoint);

void printDataMidPoint(struct MiddlePoint *midPoint);

enum RUN_STATUS checkError();
