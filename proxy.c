#include "piapi.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int piapi_sampling;

static void signal_handler(int sig)
{
	if( sig == SIGINT )
		piapi_sampling = 0;		
}

int main(int argc, char *argv[])
{
	void *cntx[256];

	signal( SIGINT, signal_handler );

	piapi_sampling = 1;
	for(unsigned int i = lowerbound; i <= upperbound; i++) {
		piapi_init( &cntx, PIAPI_MODE_PROXY, 0x0, argc, argv ); 

		piapi_collect( cntx, PIAPI_PORT_CPU, 0, 100 );
	}
	while( piapi_sampling );

	piapi_destroy( cntx );

	return 0;
}

