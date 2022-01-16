/*
 * utils.c
 *
 *  Created on: 15 січ. 2022 р.
 *      Author: vitlaiy2034
 */
#include <utils.h>

uint32_t 	get_digit_from_text(const char * i_sdigit, uint32_t *o_digit)
{
	if(i_sdigit == NULL || o_digit == NULL)
		return EINVAL;

	if(strncmp(i_sdigit, "0b", 2) == 0 && *(i_sdigit + 2) != '\0')
	{
		*o_digit = strtoul((i_sdigit + 2), NULL, 2);
	}
	else if(strncmp(i_sdigit, "0o", 2) == 0 && *(i_sdigit + 2) != '\0')
	{
		*o_digit = strtoul((i_sdigit + 2), NULL, 8);
	}
	else if(strncmp(i_sdigit, "0d", 2) == 0 && *(i_sdigit + 2) != '\0')
	{
		*o_digit = strtoul((i_sdigit + 2), NULL, 8);
	}
	else if(strncmp(i_sdigit, "0x", 2) == 0 && *(i_sdigit + 2) != '\0')
	{
		*o_digit = strtoul((i_sdigit + 2), NULL, 16);
	}
	else
	{
		*o_digit = strtoul(i_sdigit, NULL, 10);
	}
	return 0;
}

operation_t get_operation_from_text(const char * s_operation)
{
	if(strcmp(s_operation, "flip") == 0)
		return OPER_FLIP;

	if(strcmp(s_operation, "set") == 0)
		return OPER_SET;

	if(strcmp(s_operation, "unset") == 0)
		return OPER_UNSET;

	return OPER_INV;
}
radix_t get_radix_from_text(const char * s_radix)
{
	if(strcmp(s_radix, "bin") == 0)
		return RADIX_BIN;

	if(strcmp(s_radix, "oct") == 0)
		return RADIX_OCT;

	if(strcmp(s_radix, "dec") == 0)
		return RADIX_DEC;

	if(strcmp(s_radix, "hex") == 0)
		return RADIX_HEX;

	return RADIX_INV;
}

void printfBinary(uint32_t digit)
{
	while(digit != 0)
	{
		printf("%d", 0x01 & digit);
		digit = digit << 1;
	}
}
