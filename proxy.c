/* 
 * Copyright 2014 Sandia Corporation. Under the terms of Contract
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

	while( (opt=getopt( argc, argv, "a:p:t:s:f:m:lcrqvhi?" )) != -1 ) {
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
				printf( "Usage: %s [-a sa_addr] [-p sa_port] [-t sensorport]\n"
					"\t[-s samples] [-f frequency] [-m mark] [-l] [-c] [-r] [-q] [-v] [-i]\n", argv[0] );
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

	piapi_init( &cntx, PIAPI_MODE_PROXY, piapi_callback, saddr, sport, 0 ); 

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

