/* 
 * Copyright 2013-2016 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work 
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 */

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
static int quiet = 0;

static void
signal_handler(int sig)
{
	if( !quiet ) {
		switch ( sig ) {
			case SIGABRT:
				printf( "WARNING: Abnormal termination signal received\n" );
				break;
			case SIGFPE:
				printf( "WARNING: Floating point exception signal received\n" );
				break;
			case SIGILL:
				printf( "WARNING: Invalid instruction signal received\n" );
				break;
			case SIGINT:
				printf( "WARNING: Interactive attention request signal received\n" );
				break;
			case SIGSEGV:
				printf( "WARNING: Invalid memory access signal received\n" );
				break;
			case SIGTERM:
				printf( "WARNING: Termination signal received\n" );
				break;
			default:
				printf( "WARNING: Unknown signal received\n" );
				break;
		}

		printf( "WARNING: Signal caught, shutting down sampling\n" );
	}

	piapi_sampling = 0;
	sleep(1);
}

void
piapi_callback( piapi_sample_t *sample )
{
	static int header = 0;

	if( !header ) {
		if( !quiet && !verbose )
        		piapi_print_header( stdout );
        	header = 1;
        }
	piapi_print( stdout, sample, verbose );

	if( sample->total && sample->number == sample->total )
		piapi_sampling = 0;
}

int
main(int argc, char *argv[])
{
	unsigned int port = PIAPI_PORT_CPU,
		samples = 1,
		frequency = 1,
		counterfreq = 10,
		info = 0;
	int opt;
	void *cntx;

	while( (opt=getopt( argc, argv, "t:s:f:c:qvhi?" )) != -1 ) {
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
			case 'c':
				counterfreq = atoi(optarg);
				break;
			case 'q':
				quiet = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'i':
				info = 1;
				break;
			case 'h':
			case '?':
				printf( "Usage: %s [-t sensorport] [-s samples] [-f frequency] [-c counterfreq] [-q] [-v] [-i]\n", argv[0] );
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

	if( signal( SIGABRT, signal_handler ) == SIG_ERR ||
	    signal( SIGFPE, signal_handler ) == SIG_ERR ||
	    signal( SIGILL, signal_handler ) == SIG_ERR ||
	    signal( SIGINT, signal_handler ) == SIG_ERR ||
	    signal( SIGSEGV, signal_handler ) == SIG_ERR ||
	    signal( SIGTERM, signal_handler ) == SIG_ERR ) {
		if( !quiet )
			printf( "WARNING: Unable to register all signal handlers\n" );
	}

	piapi_init( &cntx, PIAPI_MODE_NATIVE, piapi_callback, 0, 0, counterfreq ); 

	piapi_sampling = 1;
	piapi_collect( cntx, (piapi_port_t)port, samples, frequency );
	while( piapi_sampling ) sched_yield();

	sleep(1);
	piapi_destroy( &cntx );

	return 0;
}

