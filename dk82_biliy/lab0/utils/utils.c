/*
 * utils.c
 *
 *  Created on: 15 січ. 2022 р.
 *      Author: vitlaiy2034
 */
#include <utils.h>

enum util_error_e get_digit_from_text(const char * i_sdigit, uint32_t *o_digit)
{
	if(i_sdigit == NULL || o_digit == NULL)
		return UTIL_ERR_INV_PARAM;

    *o_digit = strtoul(i_sdigit, NULL, 10);
	return UTIL_SUCCESS;
}


enum util_error_e 	printf_dice			(uint32_t digit)
{
    switch (digit)
	{
		case 1:
			printf("\n\n\t\t\t\t*\n");
			break;
		case 2:
			printf("\n\t\t\t\t*\n\n\t\t\t\t*");
			break;
		case 3:
			printf("\n\t\t\t\t*\n\t\t\t\t*\n\t\t\t\t*");
			break;
		case 4:
			printf("\n\t\t\t\t*    *\n\n\t\t\t\t*    *");
			break;
		case 5:
			printf("\n\t\t\t\t*   *\n\t\t\t\t  *\n\t\t\t\t*   *");
			break;
		case 6:
			printf("\n\t\t\t\t* * *\n\n\t\t\t\t* * *");
			break;
		default:
			return UTIL_ERR_INV_PARAM;
	}

	return UTIL_SUCCESS;


}
