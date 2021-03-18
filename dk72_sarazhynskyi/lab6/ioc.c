#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define IOC_MAGIC 'L'
#define SET_MOOD _IOW(IOC_MAGIC, 0, uint)

#define DEVPATH "/dev/chardev"
#define ERR(...) fprintf(stderr, "\7" __VA_ARGS__), exit(EXIT_FAILURE)
#define arr_len(X) (sizeof((X)) / sizeof([0](X)))

int main(int argc, char *argv[])
{
    int N = atoi(argv[1]);
    char _unused = '\0';
    int nfd;
    if (argc <= 3 || (1 != sscanf(argv[1], "%d%c", &nfd, &_unused)) || (_unused != '\0')) {
        ERR("Wrong arguments");
    }

    int fds[nfd] = { 0 };
    for (int i = 0; i < arr_len(fds); i++) {
        fds[i] = open(DEVPATH, O_RDWR);
        if (fds[i] < 0) {
            ERR("Open device error: %m\n");
        }
    }

    for (int i = 0; i < arr_len(fds); i++) {
        if (ioctl(fds[i], SET_MOOD, N))
            ERR("SET_MOOD error: %m\n");
    }

    for (int i = 0; i < arr_len(fds); i++)
        close(fds[i]);

    return EXIT_SUCCESS;
};
