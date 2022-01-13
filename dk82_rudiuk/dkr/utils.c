#include "utils.h"

extern enum Verbose verb;
extern enum Set_bit sb;
extern enum Set_bits sbs;
extern enum Unset_bit usb;
extern enum Unset_bits usbs;
extern enum Flip_bit fb;
extern enum Flip_bits fbs;
extern enum Radix radix;

void help_print()
{
    const char *program_version = "Version 1.0";
    const char *program_name = "Bit calculator";
    const char *program_description = "\n    The program implements a bit calculator\n\
    with the operations SET BIT / BITS, UNSET BIT / BITS, FLIP BIT / BITS.\n\
    The program accepts arguments in the form of\n\
    binary (0b...), decimal (...), hexadecimal (0x...) notation\n";

    const char *param_sb = "-sb";
    const char *param_sb_desc = "Set bit";

    const char *param_sbs = "-sbs";
    const char *param_sbs_desc = "Set bits";

    const char *param_usb = "-usb";
    const char *param_usb_desc = "Unset bit";

    const char *param_usbs = "-usbs";
    const char *param_usbs_desc = "Unset bits";

    const char *param_fb = "-fb";
    const char *param_fb_desc = "Flip bit";

    const char *param_fbs = "-fbs";
    const char *param_fbs_desc = "Flip bits";

    const char *param_verbose = "-v (--verbose)";
    const char *param_verbose_desc = "Detailed program output";

    const char *param_help = "-help (--help, help)";
    const char *param_help_desc = "View help";

    fprintf(stdout, "%s\n%s\n%s\n", program_name, program_version, program_description);
    fprintf(stdout, "%s\n\t%s\t%s\n\t%s\t%s\n\t%s\t%s\n\t%s\t%s\
                    \n\t%s\t%s\n\t%s\t%s\n\t%s\t%s\n\t%s\t%s\n",
            "Arguments:", param_sb, param_sb_desc,
            param_sbs, param_sbs_desc,
            param_usb, param_usb_desc,
            param_usbs, param_usbs_desc,
            param_fb, param_fb_desc,
            param_fbs, param_fbs_desc,
            param_verbose, param_verbose_desc,
            param_help, param_help_desc);
}

void parse_arguments(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (strcmp("-v", argv[i]) == 0 || strcmp("--verbose", argv[i]) == 0) {
            verb = VERBOSE_ENABLE;
        }
        if (strcmp("-sb", argv[i]) == 0) {
            sb = SB_ENABLE;
        }
        if (strcmp("-sbs", argv[i]) == 0) {
            sbs = SBS_ENABLE;
        }
        if (strcmp("-usb", argv[i]) == 0) {
            usb = USB_ENABLE;
        }
        if (strcmp("-usbs", argv[i]) == 0) {
            usbs = USBS_ENABLE;
        }
        if (strcmp("-fb", argv[i]) == 0) {
            fb = FB_ENABLE;
        }
        if (strcmp("-fbs", argv[i]) == 0) {
            fbs = FBS_ENABLE;
        }
        if (strcmp("--help", argv[i]) == 0 || strcmp("-help", argv[i]) == 0
                || strcmp("help", argv[i]) == 0) {
            help_print();
            exit(EXECUTION_SUCCESSFUL);
        }
    }
}

int is_number(const char *number)
{
    for (int i = 0; i < strlen(number); i++) {
        if (!isdigit(number[i]))
            return 0;
    }
    return 1;
}

void create_number(Number *A, char *argv[])
{
    A->number_str = NULL;
    A->number_str = malloc(sizeof(A->number_str) * strlen(argv[1 + verb]));
    if (NULL != A->number_str) {
        if (A->radix == DECIMAL) {
            strcpy(A->number_str, argv[1 + verb]);
            A->number = atoi(A->number_str);
        }
        else {
            strcpy(A->number_str, argv[1 + verb]);
            number_prefix_delete(A);
        }
    }

    else {
        fprintf(stderr, "Memory allocation error!\n");
        exit(MEMORY_ALLOCATION_ERROR);
    }
    if (verb)
        fprintf(stdout, "NUMBER = %s\n", A->number_str);
}

void change_bits(Number *A, int argc, char *argv[])
{
    switch (A->radix) {
    case BINARY:
        if (verb)
            fprintf(stdout, "BINARY!\n");
        break;
    case DECIMAL:
        if (verb)
            fprintf(stdout, "DECIMAL!\n");
        if (sb) {
            for (int i = 2 + verb; i < argc; i++) {
                if (strcmp("-sb", argv[i]) == 0)
                    A->number = setbit(A->number, atoi(argv[i+1]));
            }
            break;
        }
        if (usb) {
            for (int i = 2 + verb; i < argc; i++) {
                if (strcmp("-usb", argv[i]) == 0)
                    A->number = unsetbit(A->number, atoi(argv[i+1]));
            }
            break;
        }
        if (fb) {
            for (int i = 2 + verb; i < argc; i++) {
                if (strcmp("-fb", argv[i]) == 0)
                    A->number = switchbit(A->number, atoi(argv[i+1]));
            }
            break;
        }
        if (sbs) {
            for (int i = 2 + verb; i < argc; i++) {
                if (strcmp("-sbs", argv[i]) == 0) {
                    for (int j = i; j < argc - 1; j++)
                    A->number = setbit(A->number, atoi(argv[j+1]));
                }
            }
            break;
        }
        if (usbs) {
            for (int i = 2 + verb; i < argc; i++) {
                if (strcmp("-usbs", argv[i]) == 0) {
                    for (int j = i; j < argc - 1; j++)
                    A->number = unsetbit(A->number, atoi(argv[j+1]));
                }
            }
            break;
        }
        if (fbs) {
            for (int i = 2 + verb; i < argc; i++) {
                if (strcmp("-fbs", argv[i]) == 0) {
                    for (int j = i; j < argc - 1; j++)
                    A->number = switchbit(A->number, atoi(argv[j+1]));
                }
            }
            break;
        }
        break;
    case HEX:
        fprintf(stdout, "HEX!\n");
        break;
    }
}

int setbit(const int value, const int position) {
    return (value | (1 << position));
}

int unsetbit(const int value, const int position) {
    return (value & ~(1 << position));
}

int switchbit(const int value, const int position) {
    return (value ^ (1 << position));
}

void free_number(Number *A)
{
    if (NULL != A->number_str)
        free(A->number_str);
}

void number_prefix_delete(Number *A)
{
    for (int i = 0; i < strlen(A->number_str) - 2; i++)
        A->number_str[i] = A->number_str[i + 2];
    A->number_str[strlen(A->number_str) - 2] = '\0';
}

void print_number(Number *A)
{
    if (verb)
        fprintf(stdout, "New value: %d\n", A->number);
    else
        fprintf(stdout, "%d\n", A->number);
}
