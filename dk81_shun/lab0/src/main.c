#include <stdio.h>
#include <time.h>
#include <stdlib.h>

// WHO IS THE BOSS OF THIS GYM?
// Game on lab0 of Linux Embedded Course in KPI
// random (Billy, Van, Uncle Bogdan, Vladislave ..)


const char * const MY_GC[] = {
    "Donald_Trump",
    "Billy",
    "Van",
    "Uncle Bogdan",
    "VladiSlave",
    "YaroSlave"
};

int typa_rand(void)
{
	int random_var;
	
	srand(time(NULL));
	
	random_var = rand() %6;
	
	if (&random_var == NULL)
	{
	    return 0;
	}

	return random_var;
}


int main(void)
{
	int random_var = typa_rand();
	
	if(&random_var != NULL)
	{
	    printf("The boss of this gym is: %s", MY_GC[random_var]);
	} else 
	{
	    printf("Something went wrong");
	}
	return 0;
}
