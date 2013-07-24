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
	sleep(2);
	exit(0);
}

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
	unsigned int saddr = 0,
		sport = 0,
		port = PIAPI_PORT_CPU,
		samples = 0,
		frequency = 100,
		counter = 0,
		reset = 0;
	int opt;
	char *token;
	void *cntx;

	while( (opt=getopt( argc, argv, "a:p:t:s:f:cr" )) != -1 ) {
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
			case '?':
				printf( "Usage: %s [-a sa_addr] [-p sa_port] [-t sensorport]\n"
					"\t[-s samples] [-f frequency] [-c] [-r]\n", argv[0] );
				exit( -1 );
			default:
				abort( );
		} 
	}

	signal( SIGINT, signal_handler );
	piapi_init( &cntx, PIAPI_MODE_PROXY, piapi_callback, saddr, sport ); 

	if( samples ) {
		piapi_sampling = 1;
		piapi_collect( cntx, port, samples, frequency );
		while( piapi_sampling );
	}

	if( counter ) {
		piapi_sampling = 1;
		piapi_counter( cntx, port );
		while( piapi_sampling );
	}

	if( reset ) {
		piapi_sampling = 1;
		piapi_reset( cntx, port );	
		while( piapi_sampling );
	}

	piapi_destroy( &cntx );

	return 0;
}

