#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>


volatile unsigned long k_variable = 0;

typedef unsigned long type_of_k_variable;


struct V_0 {
    type_of_k_variable V;
};

void *increment(void *str)
{
    type_of_k_variable V_1 = ((struct V_0 *)str)->V;
    int i = 0;
    for (i = 0; i < V_1; i++){
        k_variable++;
	    }
}
   
int main(int argc, char *argv[])
{
    int j = 0;

    if (argc != 2) {

        fprintf(stderr, "WTF \n" );
        fprintf(stderr, " Ammount of argumens is equal to : %d \n", argc - 1);
        exit(1);
    }

    char *ggwp = NULL;
    type_of_k_variable Val1 = strtoul(argv[1], &ggwp, 10);

    struct V_0 arg_val = { .V = Val1 };

    pthread_t indiff_1;
    pthread_t indiff_2;
    pthread_create(&indiff_1, NULL, increment, &arg_val);
    pthread_create(&indiff_2, NULL, increment, &arg_val);

    for (j = 0; j < Val1; j++){
        k_variable++;
    }
    pthread_join(indiff_1, NULL);
    pthread_join(indiff_2, NULL);

    printf("expected = %ld; the resault = %ld \n", 2 * Val1, k_variable);

    return 0;
 }
