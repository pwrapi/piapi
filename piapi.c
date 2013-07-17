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
#include <arpa/inet.h>

static int piapi_debug = 0;

int
piapi_init( void **cntx, piapi_mode_t mode, piapi_callback_t callback, char argc, char **argv )
{
	int opt;
	unsigned int saddr;

	*cntx = malloc( sizeof(struct piapi_context) );
	bzero( *cntx, sizeof(struct piapi_context) );

	PIAPI_CNTX(*cntx)->mode = mode;
	PIAPI_CNTX(*cntx)->callback = callback;

	PIAPI_CNTX(*cntx)->sa_addr = PIAPI_AGNT_SADDR;
	PIAPI_CNTX(*cntx)->sa_port = PIAPI_AGNT_PORT;

	while( (opt=getopt( argc, argv, "a:p:" )) != -1 ) {
		switch( opt ) {
			case 'a':
				inet_pton( AF_INET, optarg, &saddr );
				PIAPI_CNTX(*cntx)->sa_addr = saddr;
				break;
			case 'p':
				PIAPI_CNTX(*cntx)->sa_port = atoi(optarg);
				break;
			case '?':
				printf( "Usage: %s [-a sa_addr] [-p sa_port]\n", argv[0] );
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
	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
			return piapi_native_destroy( *cntx );

		case PIAPI_MODE_PROXY:
			return piapi_proxy_destroy( *cntx );

		case PIAPI_MODE_AGENT:
			return piapi_native_destroy( *cntx );

		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	return -1;
}

int
piapi_collect( void *cntx, piapi_port_t port, unsigned int samples, unsigned int frequency )
{
	if( PIAPI_CNTX(cntx)->mode != PIAPI_MODE_AGENT ) {
		PIAPI_CNTX(cntx)->port = port;
		PIAPI_CNTX(cntx)->samples = samples;
		PIAPI_CNTX(cntx)->frequency = frequency;
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
