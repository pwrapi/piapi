/* 
 * Copyright 2013-2016 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work 
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 */

#include "piapi.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>

static int piapi_sampling;

void
piapi_callback( piapi_sample_t *sample )
{
        printf( "PIAPI (%u):\n", sample->port );
        printf( "\tsample - %u of %u\n", sample->number, sample->total );
        printf( "\ttime   - %f\n", sample->time_sec+sample->time_usec/1000000.0 );
        printf( "\tvolts  - %f\n", sample->raw.volts );
        printf( "\tamps   - %f\n", sample->raw.amps );
        printf( "\twatts  - %f\n", sample->raw.watts );

	if( sample->total && sample->number == sample->total ) {
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
	unsigned int saddr = 0,
		sport = 0,
		port;
	int opt;
	char *token;
	void *cntx;

	while( (opt=getopt( argc, argv, "a:p:" )) != -1 ) {
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
			case '?':
				printf( "Usage: %s [-a sa_addr] [-p sa_port]\n", argv[0] );
				exit( -1 );
			default:
				abort( );
		} 
	}

	piapi_init( &cntx, PIAPI_MODE_PROXY, piapi_callback, saddr, sport, 10 ); 

	piapi_sampling = 1;
	piapi_collect( cntx, PIAPI_PORT_CPU, 1000, 100 );
	while( piapi_sampling ) sched_yield();

	for( port = PIAPI_PORT_MIN; port <= PIAPI_PORT_MAX; port++ ) {
		piapi_sampling = 1;
		piapi_collect( cntx, (piapi_port_t)port, 100, 100 );
		while( piapi_sampling ) sched_yield();
	}

	piapi_reset( cntx, PIAPI_PORT_CPU );
	sleep( 2 );

	piapi_sampling = 1;
	piapi_counter( cntx, PIAPI_PORT_CPU );
	while( piapi_sampling ) sched_yield();

	sleep( 1 );
	piapi_destroy( &cntx );

	return 0;
}

