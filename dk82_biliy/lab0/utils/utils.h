/*
 * utils.h
 *
 *  Created on: 15 січ. 2022 р.
 *      Author: vitlaiy2034
 */

#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>


enum util_error_e{
	UTIL_SUCCESS 			= 0,
	UTIL_ERR_INV_PARAM
};

enum util_error_e 	get_digit_from_text(const char * i_sdigit, uint32_t *o_digit);
enum util_error_e 	printf_dice			(uint32_t digit);

#endif /* UTILS_UTILS_H_ */
