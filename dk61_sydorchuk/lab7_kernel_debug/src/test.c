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
	//fd1 = open(devname, O_RDWR);
	//printf("Opened fd of hive_dev = %d\n", fd1);

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
