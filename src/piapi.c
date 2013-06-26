#include "piapi.h"

#include "piproxy.h"
#include "piagent.h"
#include "pinative.h"

#include "piutil.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

static int piapi_debug = 0;

int
piapi_init( void **cntx, piapi_mode_t mode, piapi_callback_t callback )
{
	*cntx = malloc( sizeof(struct piapi_context) );
	bzero( *cntx, sizeof(struct piapi_context) );

	PIAPI_CNTX(*cntx)->mode = mode;
	PIAPI_CNTX(*cntx)->callback = callback;

	switch( PIAPI_CNTX(*cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
			if( piapi_debug )
        			printf( "\nPower Communication (Native)\n" );

			piapi_native_init( *cntx );
			break;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
        			printf( "\nPower Communication (Proxy <=> Agent)\n" );

			piapi_proxy_init( *cntx );

			if( piapi_debug )
       				printf( "Agent connection established\n" );
			break;

		case PIAPI_MODE_AGENT:
			if( piapi_debug )
        			printf( "\nPower Communication (Agent <=> Proxy)\n" );

			piapi_agent_init( *cntx );

			if( piapi_debug )
       				printf( "Agent listener established\n" );
			break;

		default:
			break;
	}

	return 0;
}

int
piapi_destroy( void *cntx )
{
	PIAPI_CNTX(cntx)->worker_run = 0;

	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
			piapi_native_destroy( cntx );
			break;

		case PIAPI_MODE_PROXY:
			piapi_proxy_destroy( cntx );
			break;

		case PIAPI_MODE_AGENT:
			piapi_native_destroy( cntx );
			break;

		default:
			break;
	}

	return 0;
}

int
piapi_collect( void *cntx, piapi_port_t port, unsigned int samples, unsigned int frequency )
{
	PIAPI_CNTX(cntx)->port = port;
	PIAPI_CNTX(cntx)->samples = samples;
	PIAPI_CNTX(cntx)->frequency = frequency;

	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
			if( piapi_debug )
				printf("Starting native collect\n");

			pthread_create(&(PIAPI_CNTX(cntx)->worker), 0x0, (void *)&piapi_native_thread, cntx);

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf("Starting proxy collect\n");

			piapi_proxy_collect( cntx );

			if( piapi_debug )
				printf("Stopping proxy collect\n");
			break;

		case PIAPI_MODE_AGENT:
			if( piapi_debug )
				printf("Starting agent collect\n");

			piapi_agent_collect( cntx );

			if( piapi_debug )
				printf("Stopping agent collect\n");
			break;

		default:
			break;
	}

	return 0;
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

			piapi_native_clear( cntx, port );
			break;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf("Clearing proxy counter for port %d\n", port);

			piapi_proxy_clear( cntx );
			break;

		default:
			break;
	}

	return 0;
}
