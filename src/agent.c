#include "piapi.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int main(int argc, char *argv[])
{
	void *cntx;

	piapi_init( &cntx, PIAPI_MODE_AGENT, 0x0 ); 

	piapi_collect( &cntx, PIAPI_PORT_CPU, 60, 1 );

	piapi_destroy( &cntx );
}

