#include "utils.h"

void view_help() {
    fprintf(stdout, "------------------------------------------------------------------------\n");
    fprintf(stdout, "This program accepts an arbitrary number\n");
    fprintf(stdout, "of X Y coordinates for points and displays the coordinates\n");
    fprintf(stdout, "of the central (equidistant from the rest) point.\n");
    fprintf(stdout, "Arguments:\n\t--verbose (-v)\tThe program displays ");
    fprintf(stdout, "the performed actions in detail.\n");
    fprintf(stdout, "\t--dist\t\tThe program displays the distance from an equidistant\n");
    fprintf(stdout, "\t\t\tpoint to other points.\n");
    fprintf(stdout, "\t--help\t\tView help.\n");
    fprintf(stdout, "Input example: ./dkr.o -v --dist 0 0 10 0\n");
    fprintf(stdout, "------------------------------------------------------------------------\n");
}

Array *create_array(int aNumber) {
    Array *theResult = NULL;
    if (aNumber > 0) {
        theResult = (Array *)malloc(sizeof(Array));
        if (NULL != theResult) {
            theResult->points = (Point **)malloc(sizeof(Point *)*aNumber);
                if (NULL != theResult->points) {
                    theResult->number = aNumber;
                    theResult->counter = 0;
                }
                else {
                    free(theResult);
                    theResult = NULL;
                }
        }
    }
    memset(theResult->points, 0, sizeof(Point *)*aNumber);
    return theResult;
}

void add_element(Array *anArray, Point *aPoint) {
    if (NULL != anArray && NULL != aPoint && anArray->counter < anArray->number) {
        anArray->points[anArray->counter] = create_point(aPoint);
        anArray->counter++;
    }
    else
        exit(MEMORY_ERROR);
}

Point *create_point(Point *aPoint) {
    Point *theResult = (Point *)malloc(sizeof(Point));
    if (NULL != theResult) {
        theResult->x = aPoint->x;
        theResult->y = aPoint->y;
    }
    else
        exit(MEMORY_ERROR);
    return theResult;
}

void print_array(Array *anArray) {
    int i = 0;
    if (NULL == anArray)
        exit(MEMORY_ERROR);

    //fprintf(stdout, "number(%d)\n", anArray->number);
    for (i = 0; i < anArray->counter; i++)
        print_point(anArray->points[i], i);
}

void print_point(Point *aPoint, int i) {
    if (NULL != aPoint)
        fprintf(stdout, "[Point_%d] X %f, Y %f\n", i, aPoint->x, aPoint->y);
    else
        exit(MEMORY_ERROR);
}

void destroy_point(Point *aPoint) {
    if (aPoint != NULL) {
        free(aPoint);
    }
}

void free_array(Array *anArray) {
    int i = 0;
    if (NULL != anArray) {
        for (; i < anArray->counter; i++) {
            destroy_point(anArray->points[i]);
        }
        free(anArray->points);
        free(anArray);
    }
}

void equidistant_point(Array *P) {
    if (P->counter <= 1) {
        fprintf(stderr, "%s", "Not enough data!\n");
        exit(NOT_ENOUGH_DATA);
    }
    if (P->counter == 2) {
        Point A = {(P->points[0]->x + P->points[1]->x)/2, (P->points[0]->y + P->points[1]->y)/2};
        add_element(P, &A);
        P->distance = sqrt(pow((P->points[0]->x - A.x), 2) + pow((P->points[0]->y - A.y), 2));
        return;
    }
    if (P->counter > 2) {
        double A = P->points[1]->x - P->points[0]->x;
        double B = P->points[1]->y - P->points[0]->y;
        double C = P->points[2]->x - P->points[0]->x;
        double D = P->points[2]->y - P->points[0]->y;
        double E = A * (P->points[0]->x + P->points[1]->x)
                 + B * (P->points[0]->y + P->points[1]->y);
        double F = C * (P->points[0]->x + P->points[2]->x)
                 + D * (P->points[0]->y + P->points[2]->y);
        double G = 2 * (A * (P->points[2]->y - P->points[1]->y)
                      - B * (P->points[2]->x - P->points[1]->x));

        if (G == 0) {
            fprintf(stderr, "%s", "Cannot find an equidistant point for a given set of points\n");
            exit(INVALID_POINT_SET);
        }

        double Cx = (D * E - B * F) / G;
        double Cy = (A * F - C * E) / G;
        double R = sqrt(pow((P->points[0]->x - Cx), 2) + pow((P->points[0]->y - Cy), 2));

        if (P->counter > 3) {
            for (int i = 3; i < P->counter; i++) {
                if (ceil(pow((P->points[i]->x - Cx), 2)
                       + pow((P->points[i]->y - Cy), 2)) != ceil(pow(R, 2))) {
                    fprintf(stderr, "%s",
                        "Cannot find an equidistant point for a given set of points!\n");
                    exit(INVALID_POINT_SET);
                }
            }
        }

        Point aPoint = {Cx, Cy};
        add_element(P, &aPoint);
        P->distance = R;
    }
}
