/**
 * Author: @MaksGolub

 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define CHG_BUF _IOW('V','a', unsigned long*)
#define ADD_PHR _IOW('B','b', unsigned long*)

int main(int argc, char **argv)
{
	int length, length2, fd, fd1, offset;
	char *devname = "/dev/hive_dev";
	char buffer[] = "Wow, we made these bees TWERK !";
	char buffer2[] = "Wow, we made these bees TWERK !Wow, these bees really hottest things";
	char buffer3[100];
	length = sizeof(buffer);
	length2 = sizeof(buffer2);

	fd = open(devname, O_RDWR);
	printf("Opened fd of hive_dev = %d\n", fd);
	fd1 = open(devname, O_RDWR);
	printf("Opened fd of hive_dev = %d\n", fd1);

	offset = write(fd, buffer, length);
	printf("Return from write callback, offset=%d, message=%s\n", offset, buffer);
	
	memset(buffer, 0, length);
	//lseek used cdev_lseek callback
	lseek(fd, 0, SEEK_SET);

	offset = read(fd, buffer, length);
	printf("Return from read callback, offset=%d, message=%s\n", offset, buffer);

	offset = write(fd1, buffer2, length2);
	printf("Return from write callback, offset=%d\n", offset);
	
	if (offset != -1) {
		memset(buffer2, 0, length2);
		lseek(fd1, 0, SEEK_SET);

		offset = read(fd1, buffer2, length2);
		printf("Return from read callback, offset=%d, message=%s\n", offset, buffer2);
	}
	
	if (-1 == ioctl(fd1, CHG_BUF, 100)) {
		printf("device %d : ioctl error\n", fd1);
	} else {
		printf("device %d : buffer size change to 100\n", fd1);	
		lseek(fd1, 0, SEEK_SET);
		offset = write(fd1, buffer2, length2);
		printf("Return from write callback, offset=%d\n", offset);
		
		memset(buffer2, 0, length2);
		lseek(fd1, 0, SEEK_SET);
		offset = read(fd1, buffer2, length2);
		printf("Return from read callback, offset=%d, message=%s\n", offset, buffer2);
	}

	if (-1 == ioctl(fd1, ADD_PHR, 1)) {
		printf("device %d : ioctl error\n", fd);
		
	} else {
		lseek(fd1, 0, SEEK_SET);
		offset = read(fd1, buffer3, 100);
		printf("Return from read callback, offset=%d, message=%s\n", offset, buffer3);	
	}
	
	close(fd);
	close(fd1);
	
	exit(0);
}