#include "utils.h"
#include <argp.h>

const char *argp_program_bug_address = "vitaliy2034v@gmail.com";
const char *argp_program_version 	 = "Version 1.0b";

typedef struct {
	uint32_t 	in_digit;
	bool		is_in_digit_inted;

	uint32_t 	mask;
	bool		is_mask_inited;

	operation_t operation;

	bool		vebose;

	radix_t		out_radix;
} control_block_t;

static error_t argp_parser(int __key, char *__arg, struct argp_state *__state)
{
	control_block_t *cb = ((control_block_t *)__state->input);
	switch(__key)
	{
		case 'p':
			cb->operation = get_operation_from_text(__arg);
			break;

		case 'm':
			if(get_digit_from_text(__arg, &(cb->mask)) == 0)
				cb->is_mask_inited = true;
			else
				argp_failure(__state, 1, 0, "Error in mask: %s\n", __arg);
			break;

		case 'i':
			if(get_digit_from_text(__arg, &(cb->in_digit)) == 0)
				cb->is_in_digit_inted = true;
			else
				argp_failure(__state, 1, 0, "Error in in-digit: %s\n", __arg);
			break;
		case 'r':
			cb->out_radix = get_radix_from_text(__arg);
			break;
		case 'v':
			cb->vebose = true;
			break;
		case ARGP_KEY_ARG:
			argp_failure(__state, 1, 0, "Unknown argument: %s\n", __arg);
			break;
	}
	return  0;
}


int main (int argc, char **argv)
{
	struct argp_option options[] = {
		{
			.arg   = "OPERATION",
			.doc   = "One of the operations with digits. Values could be flip, set, unset",
			.flags = 0,
			.group = 0,
			.key   = 'p',
			.name  = "operation"
		},
		{
			.arg   = "DIGIT",
			.doc   = "Mask for input digit, which defines which bits are changed. Could be represented in different radix:\n "\
					 "0x - hexadecimal, 0d(or without prefix) - decimal, 0o - octal, 0b - binary",
			.flags = 0,
			.group = 0,
			.key   = 'm',
			.name  = "mask"
		},
		{
			.arg   = "DIGIT",
			.doc   = "Input digit, on which we do operation through mask. Could be represented in different radix:\n "\
			 	 	 "0x - hexadecimal, 0d(or without prefix) - decimal, 0o - octal, 0b - binary",
			.flags = 0,
			.group = 0,
			.key   = 'i',
			.name  = "in-digit"
		},
		{
			.arg   = "RADIX",
			.doc   = "Output digit radix. Acceptable values: bin, oct, dec, hex",
			.flags = 0,
			.group = 0,
			.key   = 'r',
			.name  = "out-radix"
		},
		{
			.arg   = NULL,
			.doc   = "One of the operations with digits",
			.flags = 0,
			.group = 0,
			.key   = 'v',
			.name  = "verbose"
		},
		{0}
	};
	struct argp argp_test  = {options, argp_parser, 0, "This program can do simple binary operations"};

	control_block_t cb = {0, false, 0, false, OPER_INV, false, RADIX_DEC};

	//Check that parameters and options correctly recived
    error_t err_code = argp_parse(&argp_test, argc, argv, 0, 0, &cb);

    if(err_code)
    {
    	fprintf(stderr, "binary_calculator: Parse error: %s", strerror(err_code));
    	exit(err_code);
    }

    if(!cb.is_in_digit_inted)
    {
    	fprintf(stderr, "binary_calculator: missing input digit\n");
    	exit(EINVAL);
    }

    if(!cb.is_mask_inited)
    {
    	fprintf(stderr, "binary_calculator: missing mask\n");
    	exit(EINVAL);
    }

    uint32_t result = 0;

    switch(cb.operation)
    {
    	case OPER_FLIP:
    		result = cb.in_digit ^ cb.mask;
    		break;

    	case OPER_SET:
    		result = cb.in_digit | cb.mask;
    		break;

    	case OPER_UNSET:
    		result = cb.in_digit & ~(cb.mask);
    		break;

    	default:
    		fprintf(stderr, "binary_calculator: option corrupted\n");
    		exit(EINVAL);
    		break;
    }

    if(cb.vebose)
    	printf("Result of operation:");

    switch(cb.out_radix)
    {
    	case RADIX_BIN:
			printf("0b");
			printfBinary(result);
			break;

    	case RADIX_OCT:
			printf("0o%o", result);
			break;

    	case RADIX_DEC:
    		printf("%d", result);
    		break;

    	case RADIX_HEX:
    	    printf("0x%X", result);
    	    break;

    	case RADIX_INV:
    	default:
    		fprintf(stderr, "binary_calculator: invalid radix\n");
			exit(EINVAL);
    }
   printf("\n");
   exit(0);
}
