#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#define READ_BUFF_SIZE 100
#define TEST_NEW_BUFF_SIZE (22)
#define IOCTL_TWERK ((unsigned int)0)
#define IOCTL_SET_NEW_BUFFER_SIZE ((unsigned int)1)

static const int num_read_symbols = 16;
static const char write_str[] = "Hivemod test str 123456789\n";
static char read_buff[READ_BUFF_SIZE] = {0};

int main()
{
    int filedesc = open("/dev/hive_test_device", O_RDWR | O_APPEND);
    if(filedesc < 0)
        return 1;
 
    int write_cnt = write(filedesc,write_str, strlen(write_str));
    int read_cnt = read(filedesc, read_buff, num_read_symbols);
    printf("write(): Write bytes: expected %i, really: %i\n"
           "read(): read bytes: expected %i, really: %i\n"
           "Read buff: %s \n",
           strlen(write_str), write_cnt,
           num_read_symbols, read_cnt,
           read_buff);
    
    ioctl(filedesc, IOCTL_TWERK);
    ioctl(filedesc, IOCTL_SET_NEW_BUFFER_SIZE, TEST_NEW_BUFF_SIZE);
    read_cnt = read(filedesc, read_buff, READ_BUFF_SIZE);
    
    close(filedesc);

    printf("\nioctl: Read bytes: %i, Read buff: %s \n\n", read_cnt, read_buff);
    
    return 0;
}
