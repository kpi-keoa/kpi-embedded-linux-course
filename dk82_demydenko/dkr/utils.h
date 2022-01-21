#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* random module */
enum GEN_ERR {
    ERR_OK = 0,
    ERR_ARG = -1,
    ERR_MEM = -1,
};

typedef struct point_container {
	double x;
	double y;
    
    char n;
} point_t;

typedef struct triangle_container {
    point_t *A;
    point_t *B;
    point_t *C;
} triangle_t;

/* points */
point_t *point_create(const char ch, double x, double y);
enum GEN_ERR point_free(point_t *P);

enum GEN_ERR point_print(point_t *P);

/* triangles */
triangle_t *triangle_create(point_t *A, point_t *B, point_t *C);
enum GEN_ERR triangle_free(triangle_t *T);

enum GEN_ERR triangle_print(triangle_t *T);
point_t *triangle_center(triangle_t *T);
