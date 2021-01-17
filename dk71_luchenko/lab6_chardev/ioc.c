#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define IOC_MAGIC	'AAA'
#define SET_MOOD _IOW(IOC_MAGIC, 0, uint )

#define DEVPATH "/dev/mymod"

int main( int argc, char *argv[] ) {
	int N = atoi(argv[1]);
	int dfd;
	if ((dfd = open(DEVPATH, O_RDWR)) < 0 )
		fprintf( stderr, "\7" __VA_ARGS__ ), exit( EXIT_FAILURE );

	if (ioctl(dfd, SET_MOOD, N))
		fprintf( "SET_MOD error: %m\n" );
	close(dfd);

	return EXIT_SUCCESS;
};
