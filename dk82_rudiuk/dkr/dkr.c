#include "utils.h"

enum Verbose verb = VERBOSE_DISABLE;
enum Set_bit sb = SB_DISABLE;
enum Set_bits sbs = SBS_DISABLE;
enum Unset_bit usb = USB_DISABLE;
enum Unset_bits usbs = USBS_DISABLE;
enum Flip_bit fb = FB_DISABLE;
enum Flip_bits fbs = FBS_DISABLE;

int main(int argc, char *argv[])
{
    Number A;
    parse_arguments(argc, argv);
     if (verb) {
        for (int i = 0; i < argc; i++)
            fprintf(stdout, "argv[%d] = %s\n", i, argv[i]);
        fprintf(stdout, "argc = %d\n", argc);
    }

    if (argc == 1) {
        fprintf(stderr, "No arguments!\n");
        help_print();
        return NO_ARGUMENTS;
    }

    if (argc > 1 + verb) {
        if (strlen(argv[1 + verb]) == 1) { // if length is 1
            if (isdigit(argv[1 + verb][0]))  // and if it is digit
                A.radix = DECIMAL;        // this is decimal one-digit number
            else {                        // else this is not number
                fprintf(stderr, "Invalid data input!\n");
                return INVALID_DATA_INPUT;
            }
        }
        else if (strlen(argv[1 + verb]) > 1 && argv[1 + verb][0] == '0'
                 && argv[1 + verb][1] == 'b')
            A.radix = BINARY;
        else if (strlen(argv[1 + verb]) > 1 && argv[1 + verb][0] == '0'
                 && argv[1 + verb][1] == 'x')
            A.radix = HEX;
        else if (strlen(argv[1 + verb]) > 1 && isdigit(argv[1 + verb][0])
                 && (argv[1 + verb][0] != '0'))
            A.radix = DECIMAL;
        else {
            fprintf(stderr, "Invalid data input!\n");
            return INVALID_DATA_INPUT;
        }
    }
    else {
        fprintf(stderr, "Not enough arguments!\n");
        return NOT_ENOUGH_ARGUMENTS;
    }

    if (!((double) (argc - verb - 1 - 1) / 2 >= sb + usb + fb + sbs + usbs + fbs)
            || argc - verb - 1 - 1 == 0) {
        fprintf(stderr, "Not enough arguments or invalid argument!\n");
        return NOT_ENOUGH_ARGUMENTS;
    }

    if (sb || usb || fb) {
        for (int i = 1 + 1 + verb; i < argc; i++) {
            if (strcmp("-sb", argv[i]) == 0 || strcmp("-usb", argv[i]) == 0
                    || strcmp("-fb", argv[i]) == 0) {
                if (i < argc - 1) {
                    if (!is_number(argv[i + 1])) {
                        fprintf(stderr, "Invalid bit number!\n");
                        return INVALID_DATA_INPUT;
                    }
                }
                if  (i < argc - 2) {
                    if (is_number(argv[i + 2])) {
                        fprintf(stderr, "Too many arguments!\n");
                        return TOO_MANY_ARGUMENTS;
                    }
                }
                if (i == argc - 1) {
                    fprintf(stderr, "Not enough arguments!\n");
                    return NOT_ENOUGH_ARGUMENTS;
                }
            }
        }
    }

    if (sbs || usbs || fbs) {
        for (int i = 1 + 1 + verb; i < argc; i++) {
            if (strcmp("-sbs", argv[i]) == 0 || strcmp("-usbs", argv[i]) == 0
                    || strcmp("-fbs", argv[i]) == 0) {
                if (i == argc - 1) {
                    fprintf(stderr, "Not enough arguments!\n");
                    return NOT_ENOUGH_ARGUMENTS;
                }
                for (int j = i + 1; j < argc; j++) {
                    if (!is_number(argv[j])) {
                        fprintf(stderr, "Invalid bit number!\n");
                        return INVALID_DATA_INPUT;
                    }
                    if (j == argc - 1 && !is_number(argv[j])) {
                        fprintf(stderr, "Invalid bit number!\n");
                        return INVALID_DATA_INPUT;
                    }
                    else if (j == argc - 1)
                        break;
                    if (j < argc && (strcmp("-sbs", argv[j + 1]) == 0
                                     || strcmp("-usbs", argv[j + 1]) == 0
                                     || strcmp("-fbs", argv[j + 1]) == 0
                                     || strcmp("-sb", argv[j + 1]) == 0
                                     || strcmp("-usb", argv[j + 1]) == 0
                                     || strcmp("-fb", argv[j + 1]) == 0))
                        break;
                }
            }
        }
    }


    if (verb) {
        fprintf(stdout, "Radix: %d\n", A.radix);
    }

    create_number(&A, argv);

    change_bits(&A, argc, argv);

    print_number(&A);
    free_number(&A);
	return EXECUTION_SUCCESSFUL;
}
