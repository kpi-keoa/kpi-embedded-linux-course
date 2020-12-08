#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <string.h>

#define MAGIC_NUMB              'A'

#define IOCTL_NEW_KEY_BUF_SZ            _IOW(MAGIC_NUMB, 1, unsigned long)
#define IOCTL_NEW_DATA_BUF_SZ           _IOW(MAGIC_NUMB, 2, unsigned long)
#define IOCTL_CH_MODE                   _IOW(MAGIC_NUMB, 3, bool)

#define KEY     false
#define DATA    true



int main(int argc, char const *argv[])
{

        int fd = open("/dev/xorit", O_RDWR | O_EXCL);

        write(fd, argv[1], strlen(argv[1]));
        ioctl(fd, IOCTL_CH_MODE, KEY);

        write(fd, argv[2], strlen(argv[2]));

        char *output = malloc(100);
        read(fd, output, 100);

        printf(" %s\n", output);

        ioctl(fd, IOCTL_CH_MODE, DATA);
        write(fd, output, strlen(output));

        read(fd, output, 100);
        printf(" %s\n", output);


        close(fd);

        return 0;
}
