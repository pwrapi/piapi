#include "piapi.h"

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
	if( !verbose ) {
		printf( "%u:%u:%lu:%lu:%u:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f\n",
			sample->number, sample->total, sample->time_sec, sample->time_usec,
			sample->port, sample->raw.volts, sample->raw.amps, sample->raw.watts,
			sample->avg.volts, sample->avg.amps, sample->avg.watts,
			sample->min.volts, sample->min.amps, sample->min.watts,
			sample->max.volts, sample->max.amps, sample->max.watts,
			sample->time_total, sample->energy );
	} else {
		printf( "PIAPI:\n");
		printf( "\tsample - %u of %u\n", sample->number, sample->total );
		printf( "\ttime   - %f\n", sample->time_sec+sample->time_usec/1000000.0 );
		printf( "\tport   - %u\n", sample->port );
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
		}
	}

	if( sample->total && sample->number == sample->total )
		piapi_sampling = 0;
}

int
main(int argc, char *argv[])
{
	unsigned int saddr = 0x7f000001,
		sport = 0,
		port = PIAPI_PORT_CPU,
		samples = 1,
		frequency = 1,
		counter = 0,
		reset = 0,
		info = 0;
	int opt;
	char *token;
	void *cntx;

	while( (opt=getopt( argc, argv, "a:p:t:s:f:crvhi?" )) != -1 ) {
		switch( opt ) {
			case 'a':
				token = strtok( optarg, "." );
				saddr = atoi(token) << 24;
				token = strtok( NULL, "." );
				saddr |= ( atoi(token) << 16 );
				token = strtok( NULL, "." );
				saddr |= ( atoi(token) << 8 );
				token = strtok( NULL, "." );
				saddr |= atoi(token);
				break;
			case 'p':
				sport = atoi(optarg);
				break;
			case 't':
				port = atoi(optarg);
				break;
			case 's':
				samples = atoi(optarg);
				break;
			case 'f':
				frequency = atoi(optarg);
				break;
			case 'c':
				counter = 1;
				break;
			case 'r':
				reset = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'i':
				info = 1;
				break;
			case 'h':
			case '?':
				printf( "Usage: %s [-a sa_addr] [-p sa_port] [-t sensorport]\n"
					"\t[-s samples] [-f frequency] [-c] [-r] [-v] [-i]\n", argv[0] );
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
	piapi_init( &cntx, PIAPI_MODE_PROXY, piapi_callback, saddr, sport ); 

	if( counter ) {
		piapi_sampling = 1;
		piapi_counter( cntx, port );
		while( piapi_sampling ) sched_yield();
	} else if( reset ) {
		piapi_reset( cntx, port );	
	} else {
		piapi_sampling = 1;
		piapi_collect( cntx, port, samples, frequency );
		while( piapi_sampling ) sched_yield();
	}

	sleep( 1 );
	piapi_destroy( &cntx );

	return 0;
}

