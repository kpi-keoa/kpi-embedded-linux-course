#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>


int main()
{
	printf("Hello\n");
	
	char buf[] = "CLASSIC BEES TWERKING\n";
	const int size = sizeof(buf);
	char read_buf[size];
	memset(read_buf, 0, size);
	
	const char *devname = "/dev/hivemod";
	
	int fd = open(devname, O_RDWR);
	int fd2 = open(devname, O_RDWR);
	
	printf("FD: %i\n", fd);
	printf("FD2: %i\n", fd2);
	write(fd, buf, size);
	
	while(1)
		sleep(1);
	
	close(fd);
	close(fd2);
	
	return 0;
}
