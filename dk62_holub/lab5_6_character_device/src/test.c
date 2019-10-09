#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int length, fd, offset;
	char *devname = "/dev/hive_dev";
	char buffer[] = "TWERK!TWERK!TWERK!\n";

	length = sizeof(buffer);

	fd = open(devname, O_RDWR);
	printf("Opened fd of hive_dev = %d\n", fd);

	offset = write(fd, buffer, length);
	printf("Return from write callback, offset=%d, message=%s\n", offset, buffer);
	
	memset(buffer, 0, length);
	//lseek used cdev_lseek callback
	lseek(fd, 0, SEEK_SET);

	offset = read(fd, buffer, length);
	printf("Return from read callback, offset=%d, message=%s\n", offset, buffer);

	close(fd);
	
	exit(0);
}