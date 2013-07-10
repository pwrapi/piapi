#include "piapi.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	void *cntx;

	piapi_init( &cntx, PIAPI_MODE_PROXY, 0x0 ); 

	piapi_collect( cntx, PIAPI_PORT_CPU, 0, 100 );
	while( 1 );

	piapi_destroy( cntx );

	return 0;
}

