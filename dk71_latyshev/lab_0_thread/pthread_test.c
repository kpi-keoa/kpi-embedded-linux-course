#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>


volatile long long k_variable = 0;

typedef unsigned long k_variable_type;


struct v_0 {
    k_variable_type v;
};

void *increment(void *str)
{
    k_variable_type value_1 = ((struct v_0 *)str)->v;

    for (int i = 0; i < value_1; i++){
        k_variable++;
	    }
}
   
int main(int argc, char *argv[])
{
   
    if (argc != 2) {

    	fprintf(stderr, "WTF \n" );
        fprintf(stderr, " Ammount of argumens is equal to : %d \n", argc - 1);
        exit(EXIT_FAILURE);
    }

    char *ggwp = NULL;
    k_variable_type val1 = strtoul(argv[1], &ggwp, 10);

    struct v_0 arg_val = { .v = val1 };

    pthread_t indiff_1, indiff_2;
    pthread_create(&indiff_1, NULL, increment, &arg_val);
    pthread_create(&indiff_2, NULL, increment, &arg_val);

    for (int j = 0; j < val1; j++){
        k_variable++;
    }

    pthread_join(indiff_1, NULL);
    pthread_join(indiff_2, NULL);

    printf("expected = %ld; the resault = %Ld \n", 2 * val1, k_variable);

    return 0;
 }
