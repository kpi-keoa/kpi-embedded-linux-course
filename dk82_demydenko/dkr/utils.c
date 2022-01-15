#include "utils.h"

/* points */
point_t *point_create(const char ch, double x, double y)
{
    point_t *point = malloc(sizeof(*point));
    
    if (NULL == point)
        printf("That's bad\n");
    
    if (NULL != point) {
		point->x = x;
		point->y = y;
		point->n = ch;
        
	}
	return point;
}

enum GEN_ERR point_free(point_t *P)
{
    if (NULL != P) {
	    free(P);
        return ERR_OK;
    }
    return ERR_ARG;
}

enum GEN_ERR point_print(point_t *P)
{
	if (NULL != P) {
		printf("%c = (%lf, %lf)\n", P->n, P->x, P->y);
	    return ERR_OK;
    }
    return ERR_ARG;
}

/* triangles */
triangle_t *triangle_create(point_t *A, point_t *B, point_t *C)
{
    if (NULL == A || NULL == B || NULL == C)
        return NULL;
        
    triangle_t *triangle = malloc(sizeof(*triangle));
    
	if (NULL != triangle) {
		triangle->A = A;
		triangle->B = B;
		triangle->C = C;
	}
	
	return triangle;
}

enum GEN_ERR triangle_free(triangle_t *T)
{    
    if (NULL == T)
        return ERR_ARG;
        
    free(T);
    
    return ERR_OK;
}

point_t *triangle_center(triangle_t *T)
{
    if (NULL == T->A || NULL == T->B || NULL == T->C)
        return NULL;
    
    // dx
    double dx_AB = T->A->x - T->B->x;
    double dx_BC = T->B->x - T->C->x;
    double dx_CA = T->C->x - T->A->x;
    //dy
    double dy_AB = T->A->y - T->B->y;
    double dy_BC = T->B->y - T->C->y;
    double dy_CA = T->C->y - T->A->y;
    
    //dz
    double dz_A = powl(T->A->x, 2) + powl(T->A->y, 2);
    double dz_B = powl(T->B->x, 2) + powl(T->B->y, 2);
    double dz_C = powl(T->C->x, 2) + powl(T->C->y, 2);
    
    double zX = dx_AB * dz_C + dx_BC * dz_A + dx_CA * dz_B;
    double zY = dy_AB * dz_C + dy_BC * dz_A + dy_CA * dz_B;
    double Z = dx_AB * dy_CA - dy_AB * dx_CA;
    
    double point_x = (zX) / (2 * Z);
    double point_y = - (zY) / (2 * Z);
    
    point_t *center = point_create('O', point_x, point_y);
    
    return center;
}


enum GEN_ERR triangle_print(triangle_t *T)
{
	if (NULL != T) {
        printf("T:   ");
        
        printf("\t");
		point_print(T->A);
        printf("\t");
		point_print(T->B);
        printf("\t");
		point_print(T->C);
        
        return ERR_OK;
	}
    return ERR_ARG;
}

