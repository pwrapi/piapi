#include "piapi.h"
#include "piutil.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>

static int piapi_sampling = 0;
static int verbose = 0;

static void
signal_handler(int sig)
{
	if( sig == SIGINT )
		piapi_sampling = 0;		
	sleep(1);
	exit(0);
}

void
piapi_callback( piapi_sample_t *sample )
{
	static int header = 0;

	if( header ) piapi_print_header( );
	piapi_print( sample, verbose );

	if( sample->number && sample->number == sample->total )
		piapi_sampling = 0;
}

int
main(int argc, char *argv[])
{
	unsigned int port = PIAPI_PORT_CPU,
		samples = 1,
		frequency = 1,
		info = 0;
	int opt;
	void *cntx;

	while( (opt=getopt( argc, argv, "t:s:f:vhi?" )) != -1 ) {
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
			case 'v':
				verbose = 1;
				break;
			case 'i':
				info = 1;
				break;
			case 'h':
			case '?':
				printf( "Usage: %s [-t sensorport] [-s samples] [-f frequency] [-v] [-i]\n", argv[0] );
				exit( -1 );
			default:
				abort( );
		} 
	}

	if( info ) {
		piapi_version_t version;

		piapi_info( &version );
		printf( "PIAPI Version %u.%u.%u-r%u\n",
			version.major, version.minor, version.build, version.rev );

		return 0;
	}

	signal( SIGINT, signal_handler );
	piapi_init( &cntx, PIAPI_MODE_NATIVE, piapi_callback, 0, 0 ); 

	piapi_sampling = 1;
	piapi_collect( cntx, port, samples, frequency );
	while( piapi_sampling ) sched_yield();

	sleep( 1 );
	piapi_destroy( &cntx );

	return 0;
}

