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
piapi_init( void **cntx, piapi_mode_t mode, piapi_callback_t callback, unsigned int saddr, unsigned short sport )
{
	*cntx = malloc( sizeof(struct piapi_context) );
	bzero( *cntx, sizeof(struct piapi_context) );

	PIAPI_CNTX(*cntx)->mode = mode;
	PIAPI_CNTX(*cntx)->callback = callback;

	PIAPI_CNTX(*cntx)->sa_addr = (saddr ? saddr : PIAPI_AGNT_SADDR);
	if( piapi_debug )
		printf( "Using saddr of 0x%08x\n", PIAPI_CNTX(*cntx)->sa_addr );

	PIAPI_CNTX(*cntx)->sa_port = (sport ? sport : PIAPI_AGNT_PORT);
	if( piapi_debug )
		printf( "Using port of %u\n", PIAPI_CNTX(*cntx)->sa_port );

	switch( PIAPI_CNTX(*cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
			return piapi_native_init( *cntx );

		case PIAPI_MODE_PROXY:
			return piapi_proxy_init( *cntx );

		case PIAPI_MODE_AGENT:
			return piapi_agent_init( *cntx );

		default:
			printf( "Warning: Non-supported operation\n" );
			return -1;
	}
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
			retval = piapi_agent_destroy( *cntx );
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
piapi_counter( void *cntx, piapi_port_t port )
{
	PIAPI_CNTX(cntx)->port = port;

	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
		case PIAPI_MODE_AGENT:
			if( piapi_debug )
				printf("Retrieving counter for port %d\n", port);

			piapi_native_counter( cntx );
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
piapi_reset( void *cntx, piapi_port_t port )
{
	PIAPI_CNTX(cntx)->port = port;

	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
		case PIAPI_MODE_AGENT:
			if( piapi_debug )
				printf("Reseting counter for port %d\n", port);

			piapi_native_reset( cntx );
			return 0;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf("Reseting proxy counter for port %d\n", port);

			piapi_proxy_reset( cntx );
			return 0;

		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	return -1;
}
