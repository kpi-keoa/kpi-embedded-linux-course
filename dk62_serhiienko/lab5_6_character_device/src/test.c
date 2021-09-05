#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define CHG_BUF _IOW('D','i', unsigned long *)
#define ADD_PHR _IOW('C','k', unsigned long *)

int main(int argc, char **argv)
{
	int len, len_2, fd, offset;
	char *devname = "/dev/hive_dev";
	char buf[] = "Wow, we made these bees TWERK !";
	char buf_2[] = "Wow, we made these bees TWERK ! Let's intrude them";
	char buf_3[100];
	len = sizeof(buf);
	

	if((fd = open(devname, O_RDWR)) < 0) {
		printf("Can't open %s \n", devname);
		return 0;
	}
	offset = write(fd, buf, len);
	printf("write return: offset=%d, message=%s\n", offset, buf);
	
	memset(buf, 0, len);
	//lseek used cdev_lseek callback
	lseek(fd, 0, SEEK_SET);

	offset = read(fd, buf, len);
	printf("read return: offset=%d, message=%s\n", offset, buf);

	len_2 = sizeof(buf_2);
	if (-1 == ioctl(fd, CHG_BUF, 100)) {
		printf("ioctl error\n");
	} else {
		printf("buf size change to 100\n");	
		lseek(fd, 0, SEEK_SET);
		offset = write(fd, buf_2, len_2);
		printf("write return: offset=%d\n", offset);
		
		memset(buf_2, 0, len_2);
		lseek(fd, 0, SEEK_SET);
		offset = read(fd, buf_2, len_2);
		printf("read return: offset=%d, message=%s\n", offset, buf_2);
	}

	if (-1 == ioctl(fd, ADD_PHR, 1)) {
		printf("device %d : ioctl error\n");
		
	} else {
		lseek(fd, 0, SEEK_SET);
		offset = read(fd, buf_3, 100);
		printf("Return from read callback: offset=%d, message=%s\n", offset, buf_3);	
	}
	
	close(fd);
	exit(0);
}