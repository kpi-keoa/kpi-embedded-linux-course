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

typedef enum {
	OPER_INV,
	OPER_FLIP,
	OPER_SET,
	OPER_UNSET
} operation_t;

typedef enum {
	RADIX_INV,
	RADIX_BIN,
	RADIX_OCT,
	RADIX_DEC,
	RADIX_HEX
}radix_t;

uint32_t 	get_digit_from_text(const char * i_sdigit, uint32_t *o_digit);

operation_t get_operation_from_text	(const char * s_operation);

radix_t 	get_radix_from_text		(const char * s_radix);

void 		printfBinary			(uint32_t digit);

#endif /* UTILS_UTILS_H_ */
