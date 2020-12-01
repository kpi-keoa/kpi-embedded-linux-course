#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#define IOC_MAGIC	'L'
#define SET_MOOD _IOW(IOC_MAGIC, 0, uint )

#define DEVPATH "/dev/khmod"
#define ERR(...) fprintf( stderr, "\7" __VA_ARGS__ ), exit( EXIT_FAILURE )


int main( int argc, char *argv[] ) {
	int N = atoi(argv[1]);
	int dfd;
	if ((dfd = open(DEVPATH, O_RDWR)) < 0 )
		ERR( "Open device error: %m\n" );

	if (ioctl(dfd, SET_MOOD, N))
		ERR( "SET_MOOD error: %m\n" );
	close(dfd);

	return EXIT_SUCCESS;
};
