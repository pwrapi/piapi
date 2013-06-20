#include "piapi.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int piapi_sampling;

void
piapi_callback( struct piapi_sample *sample )
{
        printf( "PIAPI:\n");
        printf( "\tsample - %u of %u\n", sample->number, sample->total );
        printf( "\ttime   - %f\n", sample->time_sec+sample->time_usec/1000000.0 );
        printf( "\tvolts  - %f\n", sample->raw.volts );
        printf( "\tamps   - %f\n", sample->raw.amps );
        printf( "\twatts  - %f\n", sample->raw.watts );
        printf( "\tenergy - %f\n", sample->energy );

	if( sample->number == sample->total )
		piapi_sampling = 0;
}

int main(int argc, char *argv[])
{
	void *cntx;

	piapi_init( &cntx, PIAPI_MODE_NATIVE, piapi_callback ); 

	piapi_sampling = 1;
	piapi_collect( cntx, PIAPI_PORT_CPU, 1000, 100 );
	while( piapi_sampling );

	piapi_sampling = 1;
	piapi_collect( cntx, PIAPI_PORT_CPU, 1000, 100 );
	while( piapi_sampling );

	piapi_destroy( cntx );

	return 0;
}

