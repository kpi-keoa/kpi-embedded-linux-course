#include "utils.h"

int setbit(const int value, const int position) 
{
    return (value | (1 << position));
}

int unsetbit(const int value, const int position) 
{
    return (value & ~(1 << position));
}

int switchbit(const int value, const int position) 
{
    return (value ^ (1 << position));
}
