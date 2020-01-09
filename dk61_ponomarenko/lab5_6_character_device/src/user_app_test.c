#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#define BEES_TWERK _IO('a', 0)
#define SEL_BUF_SIZE _IOW('a', 1, int *)

int main(int argc, char* argv[])
{
	int fd, fd_2, num, w_r, our_buf;

	fd = open("/dev/hive_dev", O_RDWR);
	fd_2 = open("/dev/hive_dev", O_RDWR);

	char buf[] = "";

	char magic_phrase[] = "Wow, we made these bees TWERK!";
	
	if (fd_2 < 0) {
        	printf("Can't open device\n");
        	printf("fd_2 is = %d\n", fd_2);
        	return 0;
     	} else 
		printf("Open a device 2: %d\n", fd_2);


	if (fd < 0) {
        	printf("Can't open device\n");
        	printf("fd is = %d\n", fd);
        	return 0;
     	} else 
		printf("Open a device 1: %d\n", fd);

	
	int phrase_size = sizeof(magic_phrase);
	ioctl(fd_2, BEES_TWERK, &magic_phrase);
	lseek(fd_2, 0, SEEK_SET);
	char *str4magic = malloc(phrase_size);
	w_r = read(fd_2, str4magic, phrase_size);
	if (w_r < 0) {
      		printf("Failed to read the message from the device.");
      		return 0;
   	} else {
		printf ("SOME MAGIC _read_: %s\n", str4magic);
	}

	printf("\nType a buffersize and press ENTER:\n");
	scanf("%d", &num);
	getchar();

	ioctl(fd, SEL_BUF_SIZE, (int *) &num);
	printf("Type some text to write and press ENTER:\n");
	scanf("%[^\n]%*c", buf);
	int buf_size = 0;
	while (1) {
		if (buf[buf_size] != '\0')
			buf_size++;
		else
			break;
	}
	buf_size++;
	
	printf("buf is: %d\n", buf_size);
	our_buf = write(fd, buf, buf_size);
	if (our_buf < 0) {
      		printf("Failed to write the message to the device `our_buf`.");
      		return 0;
   	} else 
		printf ("Written line: %s\n", buf);
	
	memset(buf, 0, buf_size);
	lseek(fd, 0, SEEK_SET);
	printf("Press ENTER to to read!");
	getchar();
    	
	our_buf = read(fd, buf, buf_size);
	if (our_buf < 0){
      		printf("Failed to read the message from the device `our_buf`.");
      		return 0;
   	} else
		printf ("Read line: %s\n", buf);
/*
	int dv[10];
	for (int i = 0; i < 10; i ++) {
		dv[i] = open("/dev/hive_dev", O_RDWR);
	}

	for (int i = 0; i < 10; i ++) {
		close(dv[i]);
	}
*/
	return 0;
}

