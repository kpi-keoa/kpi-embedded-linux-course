#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdbool.h>

#define SET_MAXLENGTH   _IOW('i', 0, uint)
#define SETMODE         _IOW('i', 1, bool)
#define ENCODE          false
#define DECODE          true

int main(int argc, char const *argv[])
{
    const char c[] = "heil hitler, heil mussolini";

    int fd = open("/dev/cbase64", O_RDWR | O_EXCL);

    ioctl(fd, SET_MAXLENGTH, (uint)100);
    ioctl(fd, SETMODE, ENCODE);

    write(fd, c, sizeof(c));
    char *output = malloc(100);
    uint rb = read(fd, output, 100);
    printf("%s, got %d bytes\n", output, rb);

    ioctl(fd, SETMODE, DECODE);
    write(fd, output, strlen(output) + 1);
    memset(output, 0, 100);
    rb = read(fd, output, 100);
    printf("%s, got %d bytes\n", output, rb);

    close(fd);

    return 0;
}
