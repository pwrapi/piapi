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

int piapi_sampling;

void
piapi_callback( piapi_sample_t *sample )
{
	printf( "%u:%u:%lu:%lu:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f\n",
		sample->number, sample->total, sample->time_sec, sample->time_usec,
		sample->raw.volts, sample->raw.amps, sample->raw.watts,
		sample->avg.volts, sample->avg.amps, sample->avg.watts,
		sample->min.volts, sample->min.amps, sample->min.watts,
		sample->max.volts, sample->max.amps, sample->max.watts,
		sample->time_total, sample->energy );

	if( sample->number == sample->total )
		piapi_sampling = 0;
}

int main(int argc, char *argv[])
{
	void *cntx;

	signal( SIGINT, signal_handler );
	piapi_init( &cntx, PIAPI_MODE_PROXY, piapi_callback, argc, argv ); 

	piapi_sampling = 1;
	piapi_collect( cntx, 0, 0, 0 );
	while( piapi_sampling );

	piapi_destroy( &cntx );

	return 0;
}

