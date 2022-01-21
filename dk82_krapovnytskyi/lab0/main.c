#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static const char *const USAGE = ("Usage:\n"
                                  "  %s [NUMBER]...\n");

typedef uint8_t byte;

typedef struct {
    byte result;
    char name[10];
} player;


int main(int argc, char* argv[])
{

    byte rnd_arr[argc - 1];
    player arr[argc - 1];

    if (argc <= 1) {
        fprintf(stderr, USAGE, argv[0]);
        goto fail;
    }

    const char pth_urandom[] = "/dev/urandom";
    FILE* rand_file = fopen(pth_urandom, "rb");

    if (rand_file == NULL) {
        fprintf(stderr,"fopen() failed in file %s at line # %d\n", __FILE__, __LINE__);
        goto fail;
    }

    fread(rnd_arr, sizeof(byte), sizeof(rnd_arr), rand_file);

    for (long i = 0; i < argc - 1; i++) {
        strcpy(arr[i].name, argv[i + 1]);
        arr[i].result = (rnd_arr[i] % 6) + 1;
        printf("Player's %s resuls is: %d \n", arr[i].name, arr[i].result);
    }

    fclose(rand_file);

    return 0;

    fail:
        exit(EXIT_FAILURE);
}
