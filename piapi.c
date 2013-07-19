#include "piapi.h"

#include "piproxy.h"
#include "piagent.h"
#include "pinative.h"

#include "piutil.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#ifndef PIAPI_DEBUG
static int piapi_debug = 0;
#else
static int piapi_debug = 1;
#endif

int
piapi_init( void **cntx, piapi_mode_t mode, piapi_callback_t callback, char argc, char **argv )
{
	int opt;
	char *token;
	unsigned int saddr;

	*cntx = malloc( sizeof(struct piapi_context) );
	bzero( *cntx, sizeof(struct piapi_context) );

	PIAPI_CNTX(*cntx)->mode = mode;
	PIAPI_CNTX(*cntx)->callback = callback;

	PIAPI_CNTX(*cntx)->sa_addr = PIAPI_AGNT_SADDR;
	PIAPI_CNTX(*cntx)->sa_port = PIAPI_AGNT_PORT;
	PIAPI_CNTX(*cntx)->port = PIAPI_PORT_CPU;
	PIAPI_CNTX(*cntx)->samples = 1;
	PIAPI_CNTX(*cntx)->frequency = 100;

	while( (opt=getopt( argc, argv, "a:p:t:s:f:" )) != -1 ) {
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

				PIAPI_CNTX(*cntx)->sa_addr = saddr;
				if( piapi_debug )
					printf( "Using saddr of 0x%08x\n", PIAPI_CNTX(*cntx)->sa_addr );
				break;
			case 'p':
				PIAPI_CNTX(*cntx)->sa_port = atoi(optarg);
				if( piapi_debug )
					printf( "Using port of %u\n", PIAPI_CNTX(*cntx)->sa_port );
				break;
			case 't':
				PIAPI_CNTX(*cntx)->port = atoi(optarg);
				break;
			case 's':
				PIAPI_CNTX(*cntx)->samples = atoi(optarg);
				break;
			case 'f':
				PIAPI_CNTX(*cntx)->frequency = atoi(optarg);
				break;
			case '?':
				printf( "Usage: %s [-a sa_addr] [-p sa_port] [-t sensorport] [-s samples] [-f frequency]\n", argv[0] );
				exit( -1 );
			default:
				abort( );
		} 
	}

	switch( PIAPI_CNTX(*cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
			return piapi_native_init( *cntx );

		case PIAPI_MODE_PROXY:
			return piapi_proxy_init( *cntx );

		case PIAPI_MODE_AGENT:
			return piapi_agent_init( *cntx );

		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	return -1;
}

int
piapi_destroy( void **cntx )
{
	int retval = -1;

	switch( PIAPI_CNTX(*cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
			retval = piapi_native_destroy( *cntx );
			break;

		case PIAPI_MODE_PROXY:
			retval = piapi_proxy_destroy( *cntx );
			break;

		case PIAPI_MODE_AGENT:
			retval = piapi_native_destroy( *cntx );
			break;

		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	free( *cntx );
	*cntx = 0x0;

	return retval;
}

int
piapi_collect( void *cntx, piapi_port_t port, unsigned int samples, unsigned int frequency )
{
	if( PIAPI_CNTX(cntx)->mode != PIAPI_MODE_AGENT ) {
		if( port ) PIAPI_CNTX(cntx)->port = port;
		if( samples ) PIAPI_CNTX(cntx)->samples = samples;
		if( frequency ) PIAPI_CNTX(cntx)->frequency = frequency;
	}

	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
			return piapi_native_collect( cntx );

		case PIAPI_MODE_PROXY:
			return piapi_proxy_collect( cntx );

		case PIAPI_MODE_AGENT:
		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	return -1;
}

int
piapi_counter( void *cntx, piapi_port_t port, piapi_sample_t *sample )
{
	PIAPI_CNTX(cntx)->port = port;

	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
		case PIAPI_MODE_AGENT:
			if( piapi_debug )
				printf("Retrieving counter for port %d\n", port);

			piapi_native_counter( cntx, port, sample );
			break;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf("Retrieving proxy counter for port %d\n", port);

			piapi_proxy_counter( cntx );
			break;

		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	return 0;
}

int
piapi_clear( void *cntx, piapi_port_t port )
{
	PIAPI_CNTX(cntx)->port = port;

	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
		case PIAPI_MODE_AGENT:
			if( piapi_debug )
				printf("Clearing counter for port %d\n", port);

			piapi_native_clear( cntx );
			return 0;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf("Clearing proxy counter for port %d\n", port);

			piapi_proxy_clear( cntx );
			return 0;

		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	return -1;
}
