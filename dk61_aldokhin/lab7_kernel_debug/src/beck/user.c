#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEVNAM "test23"
#define LEN_MSG 1024

char dev[ 80 ] = "/dev/test231";

int prepare( char *test ) {
   int df;
   if( ( df = open( dev, O_RDWR ) ) < 0 )
      printf( "open device error: %m\n" );
   int res, len = strlen( test );
   if( ( res = write( df, test, len ) ) != len )
      printf( "write device error: %m\n" );
   else 
      printf( "prepared %d bytes: %s\n", res, test );
   return df;
}

void test( int df ) {
   char buf[ LEN_MSG + 1 ];
   int res;
   printf( "------------------------------------\n" );
   do {
      if( ( res = read( df, buf, LEN_MSG ) ) > 0 ) {
         buf[ res ] = '\0';
         printf( "read %d bytes: %s\n", res, buf );
      }
      else if( res < 0 )
         printf( "read device error: %m\n" );
      else 
         printf( "read end of stream\n" );
   } while ( res > 0 );
   printf( "------------------------------------\n" );
}

int main( int argc, char *argv[] ) {
   //strcat( dev, DEVNAM );
   int df1, df2;                   // разные дескрипторы одного устройства 
   df1 = prepare( "1111111" );
   df2 = prepare( "22222" );
   test( df1 );
   test( df2 );
//   release( dev, df1 );
close( df1);   
	close( df2);
   return EXIT_SUCCESS;
};
