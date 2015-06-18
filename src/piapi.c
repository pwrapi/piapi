/* 
 * Copyright 2013-2015 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work 
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 */

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

#ifndef USE_DEBUG
static int piapi_debug = 0;
#else
static int piapi_debug = 1;
#endif

int
piapi_init( void **cntx, piapi_mode_t mode, piapi_callback_t callback,
    unsigned int saddr, unsigned short sport, unsigned int counterfreq )
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

        PIAPI_CNTX(*cntx)->frequency = counterfreq;

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
piapi_halt( void *cntx, piapi_port_t port )
{
	PIAPI_CNTX(cntx)->port = port;

	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
		case PIAPI_MODE_AGENT:
			if( piapi_debug )
				printf( "Halting collection for port %d\n", port );

			piapi_native_halt( cntx );
			return 0;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf( "Halting proxy collection for port %d\n", port );

			piapi_proxy_halt( cntx );
			return 0;

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
				printf( "Retrieving counter for port %d\n", port );

			piapi_native_counter( cntx );
			break;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf( "Retrieving proxy counter for port %d\n", port );

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
				printf( "Reseting counter for port %d\n", port );

			piapi_native_reset( cntx );
			break;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf( "Reseting proxy counter for port %d\n", port );

			piapi_proxy_reset( cntx );
			break;

		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	return 0;
}

int
piapi_log( void *cntx, piapi_port_t port, unsigned int frequency )
{
	PIAPI_CNTX(cntx)->port = port;
	PIAPI_CNTX(cntx)->frequency = frequency;

	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
		case PIAPI_MODE_AGENT:
			if( piapi_debug )
				printf( "Controlling counter log for port %d to %u\n", port, frequency );

			piapi_native_log( cntx );
			break;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf( "Controlling proxy counter log for port %d to %u\n", port, frequency );

			piapi_proxy_log( cntx );
			break;

		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	return 0;
}

int
piapi_mark( void *cntx, char *marker )
{
	strcpy( PIAPI_CNTX(cntx)->command, marker );

	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
		case PIAPI_MODE_AGENT:
			if( piapi_debug )
				printf( "Marking counter log with %s\n", marker );

			piapi_native_mark( cntx );
			break;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf( "Marking proxy log with %s\n", marker );

			piapi_proxy_mark( cntx );
			break;

		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	return 0;
}

int
piapi_train( void *cntx, piapi_port_t port )
{
	PIAPI_CNTX(cntx)->port = port;

	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
		case PIAPI_MODE_AGENT:
			if( piapi_debug )
				printf( "Training counter for port %d\n", port );

			piapi_native_train( cntx );
			break;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf( "Training proxy counter for port %d\n", port );

			piapi_proxy_train( cntx );
			break;

		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	return 0;
}

int
piapi_detect( void *cntx, piapi_port_t port )
{
	PIAPI_CNTX(cntx)->port = port;

	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
		case PIAPI_MODE_AGENT:
			if( piapi_debug )
				printf( "Detecting counter for port %d\n", port );

			piapi_native_detect( cntx );
			break;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf( "Detecting proxy counter for port %d\n", port );

			piapi_proxy_detect( cntx );
			break;

		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	return 0;
}

int
piapi_predict( void *cntx )
{
	switch( PIAPI_CNTX(cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
		case PIAPI_MODE_AGENT:
			if( piapi_debug )
				printf( "Predicting counter\n" );

			piapi_native_predict( cntx );
			break;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
				printf( "Predicting proxy counter\n" );

			piapi_proxy_predict( cntx );
			break;

		default:
			printf( "Warning: Non-supported operation\n" );
			break;
	}

	return 0;
}

int
piapi_info( piapi_version_t *version )
{
	char rev[80] = PIAPI_REV_STR;

	version->major = PIAPI_MAJOR;
	version->minor = PIAPI_MINOR;
	version->build = PIAPI_BUILD;
	version->rev = atoi( strchr( rev, ':' ) + 1 );
	
	if( piapi_debug )
		printf( "PIAPI version %u.%u.%u-r%u\n",
			version->major, version->minor, version->build, version->rev );

	return 0;
}
