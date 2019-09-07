#include <stdio.h>
#include "../inc/header.h"

void *func(void *arg)
{
	for(int i = 0; i < 100000000; i++){
		global_counter++;
	}
	return NULL;
}