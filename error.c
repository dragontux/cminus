#include "error.h"

void die( int code, char *fmt, ... ){
	va_list ap;
	va_start( ap, fmt );

	printf( "Error %d: ", code );
	vprintf( fmt, ap );

	va_end( ap );
	exit( code );
}

