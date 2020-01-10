//Some algoritms were taken from the Yaroslav Sokol
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define LENGTH _IOW('i', 0, int *)
#define BUFFER _IOW('i', 1, char *)
int main()
{
	char *devname = "/dev/hive_dev";
	int f1, f2, offset;
	char magic_p[] = "Wow, we made these bees TWERK !";
	char test_buf[100], test_buf1[100];

	f1 = open(devname, O_RDWR);
	printf("Opened file = %d\n", f1);

	offset = write(f1, magic_p, sizeof(magic_p));

	lseek(f1, 0, SEEK_SET);
	offset = read(f1, test_buf, 2 * sizeof(magic_p));
	printf("result1 = %s | size = %i\n", test_buf, strlen(test_buf));

	lseek(f1, 0, SEEK_SET);
	lseek(f1, 5, SEEK_CUR);
	offset = read(f1, test_buf, 2 * sizeof(magic_p));
	printf("result2 = %s | size = %i\n", test_buf, strlen(test_buf));

	lseek(f1, -10, SEEK_END);
	offset = read(f1, test_buf, 2 * sizeof(magic_p));
	printf("result3 = %s | size = %i\n", test_buf, strlen(test_buf));

	close(f1);

	f2 = open(devname, O_RDWR);
	printf("Opened file = %d\n", f2);

	lseek(f2, 0, SEEK_SET);
	ioctl(f2, LENGTH, 50);
	lseek(f2, 0, SEEK_SET);
	ioctl(f2, BUFFER, &magic_p);

	lseek(f2, 0, SEEK_SET);
	offset = read(f2, test_buf1, sizeof(magic_p));
	printf("result4 = %s | size = %i\n", test_buf1, strlen(test_buf1));

	close(f2);

	return 0;
}

