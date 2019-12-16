#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define IOCTL_BEE_TWERK 0
#define IOCTL_CHG_BUF_SIZE 1

int main()
{
	printf("Hello\n");
	
	char buf[] = "My twerk letters My twerk letters My twerk letters My twerk letters My twerk letters";
	const int size = sizeof(buf);
	char read_buf[size];
	memset(read_buf, 0, size);
	
	const char *devname = "/dev/hivemod";
	
	int fd = open(devname, O_RDWR);
	int fd2 = open(devname, O_RDWR);
	int fd3 = open(devname, O_RDWR);
	printf("FD: %i\n", fd);
	printf("FD2: %i\n", fd2);
	printf("FD3: %i\n", fd3);
	
	ioctl(fd, IOCTL_CHG_BUF_SIZE, 85);
	write(fd, buf, size);
	read(fd, read_buf, size);
	printf("BORING READ BUF before super twerk: %s\n", read_buf);
	ioctl(fd, IOCTL_BEE_TWERK, 0);
	read(fd, read_buf, size);
	
	close(fd);
	//close(fd2);
	//close(fd3);
	printf("READ BUF REALLY TWERK??: %s\n", read_buf);
	
	
	return 0;
}
