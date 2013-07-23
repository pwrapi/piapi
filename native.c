#include "piapi.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int piapi_sampling;

static void signal_handler(int sig)
{
	if( sig == SIGINT )
		piapi_sampling = 0;		
}

void
piapi_callback( piapi_sample_t *sample )
{
        printf( "PIAPI:\n");
        printf( "\tsample - %u of %u\n", sample->number, sample->total );
        printf( "\ttime   - %f\n", sample->time_sec+sample->time_usec/1000000.0 );
        printf( "\tvolts  - %f\n", sample->raw.volts );
        printf( "\tamps   - %f\n", sample->raw.amps );
        printf( "\twatts  - %f\n", sample->raw.watts );

	if( sample->number == sample->total ) {
        	printf( "PIAPI Summary:\n");
	        printf( "\tavg volts    - %f\n", sample->avg.volts );
        	printf( "\tavg amps     - %f\n", sample->avg.amps );
      	  	printf( "\tavg watts    - %f\n", sample->avg.watts );

	        printf( "\tmin volts    - %f\n", sample->min.volts );
        	printf( "\tmin amps     - %f\n", sample->min.amps );
      	  	printf( "\tmin watts    - %f\n", sample->min.watts );

	        printf( "\tmax volts    - %f\n", sample->max.volts );
        	printf( "\tmax amps     - %f\n", sample->max.amps );
      	  	printf( "\tmax watts    - %f\n", sample->max.watts );

	        printf( "\ttotal time   - %f\n", sample->time_total );
	        printf( "\ttotal energy - %f\n", sample->energy );

		piapi_sampling = 0;
	}
}

int main(int argc, char *argv[])
{
	unsigned int port = PIAPI_PORT_CPU,
		samples = 0,
		frequency = 0;
	int opt;
	void *cntx;

	while( (opt=getopt( argc, argv, "a:p:t:s:f:c" )) != -1 ) {
		switch( opt ) {
			case 't':
				port = atoi(optarg);
				break;
			case 's':
				samples = atoi(optarg);
				break;
			case 'f':
				frequency = atoi(optarg);
				break;
			case '?':
				printf( "Usage: %s [-t sensorport] [-s samples] [-f frequency]\n", argv[0] );
				exit( -1 );
			default:
				abort( );
		} 
	}

	signal( SIGINT, signal_handler );
	piapi_init( &cntx, PIAPI_MODE_NATIVE, piapi_callback, 0, 0 ); 

	if( samples ) {
		piapi_sampling = 1;
		piapi_collect( cntx, port, samples, frequency );
		while( piapi_sampling );
	}

	piapi_destroy( &cntx );

	return 0;
}

