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
		log = 0,
		counter = 0,
		reset = 0,
		info = 0;
	int opt;
	char *token, *mark = "";
	void *cntx;

	while( (opt=getopt( argc, argv, "a:p:t:s:f:m:lcrvhi?" )) != -1 ) {
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
			case 'm':
				mark = strtok( optarg, "" );
				break;
			case 'l':
				log = 1;
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
					"\t[-s samples] [-f frequency] [-m mark] [-l] [-c] [-r] [-v] [-i]\n", argv[0] );
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
	} else if( log ) {
		piapi_log( cntx, port, frequency );
	} else if( strcmp( mark, "" ) != 0 ) {
		piapi_mark( cntx, mark );
	} else {
		piapi_sampling = 1;
		piapi_collect( cntx, port, samples, frequency );
		while( piapi_sampling ) sched_yield();
	}

	sleep( 1 );
	piapi_destroy( &cntx );

	return 0;
}

