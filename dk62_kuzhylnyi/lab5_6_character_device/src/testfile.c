/* it is only test operations */

#include <stdio.h>
#include <sys/types.h>

#include <unistd.h>
#include <sys/types.h>

#include <sys/stat.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#define BUFFER_REALLOC _IOW('C', 1, unsigned long*)
#define ADD_MAGIC_PHRASE _IO('H', 2)


static char *buffer = "Hey you, How are you? Im fine thanks bro. Bye";
static char *buffer_small = "Honey, can you please cum to me?";

char rd_bigbuf[64];
char rd_halfbuf[32];

char ttemp_buffer[128];

int main(void) {
        

      //  if (0 == ioctl(fd, BUFFER_REALLOC, 128) ) {

        //}
                /*
        int fd2 = open("/dev/hive", O_RDWR);
                printf("Open op\n");

        if (fd2 < 0) {

                printf("error open \n");
        }
        write(fd2, buffer_small, 32);
        printf(" Write op: %s\n", buffer_small);

        lseek(fd2, 0, SEEK_SET);   
        printf(" Teleport to start\n");

        read(fd2, rd_halfbuf, 32);
        printf(" Read op: %s\n", rd_halfbuf);
        */
        
        int fd = open("/dev/hive", O_RDWR);

        if (fd < 0) {

                printf("error open \n");
        }     
        write(fd, buffer_small, sizeof(buffer_small)*100);
        printf(" Write op: %s\n", buffer_small);


        if (0 == ioctl(fd, ADD_MAGIC_PHRASE) ) {

        } 
        
       unsigned long new_size = 64;
      //  int ret = ioctl(fd, BUFFER_REALLOC, (unsigned long *)&new_size);
       // printf("ioctl ret:%d", ret);
                       lseek(fd, 0, SEEK_SET);        

        read(fd, ttemp_buffer, 1024);
       printf("appended: %s \n", ttemp_buffer);
        
        
        close(fd);
      //  close(fd2);
        printf("Close op\n");

}
