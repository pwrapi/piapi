/* 
 * Copyright 2014 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work 
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 */

#include "piproxy.h"
#include "piutil.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>

#ifndef PIAPI_PROXY_DEBUG
static int piapi_proxy_debug = 0;
#else
static int piapi_proxy_debug = 1;
#endif

static int 
piapi_proxy_parse( char *buf, unsigned int len, piapi_sample_t *sample )
{
	char *token;

	if( piapi_proxy_debug )
		printf( "Parsing proxy message %s\n", buf );

	if( (token = strtok( buf, ":" )) == NULL)
		return -1;
	sample->number = atoi(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->total = atoi(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->time_sec = atol(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->time_usec = atol(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->port = atoi(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->raw.volts = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->raw.amps = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->raw.watts = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->avg.volts = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->avg.amps = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->avg.watts = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->min.volts = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->min.amps = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->min.watts = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->max.volts = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->max.amps = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->max.watts = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->time_total = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->energy = atof(token);

	if( piapi_proxy_debug ) {
		printf( "\tsample     - %u of %u\n", sample->number, sample->total );
		printf( "\ttime       - %f\n", sample->time_sec+sample->time_usec/1000000.0 );
		printf( "\tport       - %u\n", sample->port );
		printf( "\traw volts  - %f\n", sample->raw.volts );
		printf( "\traw amps   - %f\n", sample->raw.amps );
		printf( "\traw watts  - %f\n", sample->raw.watts );
		printf( "\tavg volts  - %f\n", sample->avg.volts );
		printf( "\tavg amps   - %f\n", sample->avg.amps );
		printf( "\tavg watts  - %f\n", sample->avg.watts );
		printf( "\tmin volts  - %f\n", sample->min.volts );
		printf( "\tmin amps   - %f\n", sample->min.amps );
		printf( "\tmin watts  - %f\n", sample->min.watts );
		printf( "\tmax volts  - %f\n", sample->max.volts );
		printf( "\tmax amps   - %f\n", sample->max.amps );
		printf( "\tmax watts  - %f\n", sample->max.watts );
		printf( "\ttime total - %f\n", sample->time_total );
		printf( "\tenergy     - %f\n", sample->energy );
	} 

	return 0;
}

static void
piapi_proxy_thread( void *cntx )
{
	piapi_sample_t sample;
	char buf[ PIAPI_BUF_SIZE ];
	ssize_t rc;

	sample.cntx = cntx;

	PIAPI_CNTX(cntx)->worker_run = 1;
	while( PIAPI_CNTX(cntx)->worker_run ) {
		rc = 0;
		do {
			rc += read( PIAPI_CNTX(cntx)->fd, buf+rc, 1 );
			if( rc <= 0 ) {
				sched_yield();
				continue;
			}
		} while( rc && buf[rc-1] != ';' );

		buf[rc] = '\0';
		while( rc > 0 ) {
			if( !isspace( buf[rc-1] ) )
				break;
			buf[--rc] = '\0';
		}

		if( PIAPI_CNTX(cntx)->callback ) {
			piapi_proxy_parse( buf, rc, &sample );
			PIAPI_CNTX(cntx)->callback( &sample );
		}
	}
}

static int
piapi_proxy_connect( void *cntx )
{
	struct sockaddr_in addr;
	ssize_t rc;

	if( piapi_proxy_debug )
		printf( "Establishing agent connection with %u.%u.%u.%u on port %u\n",
			*((char *)(&(PIAPI_CNTX(cntx)->sa_addr))+3),
			*((char *)(&(PIAPI_CNTX(cntx)->sa_addr))+2),
			*((char *)(&(PIAPI_CNTX(cntx)->sa_addr))+1),
			*((char *)(&(PIAPI_CNTX(cntx)->sa_addr))+0),
			PIAPI_CNTX(cntx)->sa_port );

        PIAPI_CNTX(cntx)->fd = socket( PF_INET, SOCK_STREAM, 0 );
        if( PIAPI_CNTX(cntx)->fd < 0 ) {
                printf( "ERROR: socket() failed! rc=%d\n", PIAPI_CNTX(cntx)->fd );
                return -1;
        }

	bzero((void *)&addr, sizeof(addr));
        addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl( PIAPI_CNTX(cntx)->sa_addr );
	addr.sin_port = htons( PIAPI_CNTX(cntx)->sa_port );

	rc = connect( PIAPI_CNTX(cntx)->fd, (struct sockaddr *) &addr, sizeof(addr) );
	if( rc < 0 ) {
		printf( "ERROR: connect() failed!\n" );
		perror( "connect" );
		return -1;
	}

	if( piapi_proxy_debug )
		printf( "Agent IP address is %d.%d.%d.%d\n",
			*((char *)(&addr.sin_addr.s_addr)+0),
			*((char *)(&addr.sin_addr.s_addr)+1),
			*((char *)(&addr.sin_addr.s_addr)+2),
			*((char *)(&addr.sin_addr.s_addr)+3) );

	if( piapi_proxy_debug )
        	printf( "Connected to agent port %d\n", PIAPI_CNTX(cntx)->sa_port );

	return 0;
}

int
piapi_proxy_collect( void *cntx )
{
	char buf[ PIAPI_BUF_SIZE ] = "";
	unsigned int len;

	if( piapi_proxy_debug )
		printf( "Setting agent to collect on sensor port %u\n", PIAPI_CNTX(cntx)->port);

	strcpy( PIAPI_CNTX(cntx)->command, "collect" );
	len = sprintf( buf, "%s:%u:%u:%u;", PIAPI_CNTX(cntx)->command,
		PIAPI_CNTX(cntx)->port, PIAPI_CNTX(cntx)->samples, PIAPI_CNTX(cntx)->frequency );

	if( writen( PIAPI_CNTX(cntx)->fd, buf, len ) < 0 ) {
		printf("Error while attempting to initiate collection\n");
		return -1;
	}

	if( piapi_proxy_debug )
		printf( "Successfully started collect\n");

	return 0;
}

int
piapi_proxy_halt( void *cntx )
{
	char buf[ PIAPI_BUF_SIZE ] = "";
	unsigned int len;

	if( piapi_proxy_debug )
		printf( "Requesting agent to halt collection on sensor port %u\n", PIAPI_CNTX(cntx)->port);

	strcpy( PIAPI_CNTX(cntx)->command, "halt" );
	len = sprintf( buf, "%s:%u;", PIAPI_CNTX(cntx)->command, PIAPI_CNTX(cntx)->port );

	if( writen( PIAPI_CNTX(cntx)->fd, buf, len ) < 0 ) {
		printf("Error while attempting to halt collection\n");
		return -1;
	}

	if( piapi_proxy_debug )
		printf( "Successfully halted collect\n");

	return 0;
}

int
piapi_proxy_counter( void *cntx )
{
	char buf[ PIAPI_BUF_SIZE ] = "";
	unsigned int len;

	if( piapi_proxy_debug )
		printf( "Querying agent to get counter on sensor port %u\n", PIAPI_CNTX(cntx)->port);

	strcpy( PIAPI_CNTX(cntx)->command, "counter" );
	len = sprintf( buf, "%s:%u;", PIAPI_CNTX(cntx)->command, PIAPI_CNTX(cntx)->port );

	if( writen( PIAPI_CNTX(cntx)->fd, buf, len ) < 0 ) {
		printf( "Error while attempting to request counter\n" );
		return -1;
	}

	if( piapi_proxy_debug )
		printf( "Successfully queried counter\n");

	return 0;
}

int
piapi_proxy_reset( void *cntx )
{
	char buf[ PIAPI_BUF_SIZE ] = "";
	unsigned int len;

	if( piapi_proxy_debug )
		printf( "Requesting agent to reset counter on sensor port %u\n", PIAPI_CNTX(cntx)->port);

	strcpy( PIAPI_CNTX(cntx)->command, "reset" );
	len = sprintf( buf, "%s:%u;", PIAPI_CNTX(cntx)->command, PIAPI_CNTX(cntx)->port );

	if( writen( PIAPI_CNTX(cntx)->fd, buf, len ) < 0 ) {
		printf("Error while attempting to reset counter\n");
		return -1;
	}

	if( piapi_proxy_debug )
		printf( "Successfully reset counter\n");

	return 0;
}

int
piapi_proxy_log( void *cntx )
{
	char buf[ PIAPI_BUF_SIZE ] = "";
	unsigned int len;

	if( piapi_proxy_debug )
		printf( "Controlling agent log counter frequency to %u on sensor port %u\n",
			PIAPI_CNTX(cntx)->frequency, PIAPI_CNTX(cntx)->port );

	strcpy( PIAPI_CNTX(cntx)->command, "log" );
	len = sprintf( buf, "%s:%u:%u;", PIAPI_CNTX(cntx)->command,
		PIAPI_CNTX(cntx)->port, PIAPI_CNTX(cntx)->frequency );

	if( writen( PIAPI_CNTX(cntx)->fd, buf, len ) < 0 ) {
		printf( "Error while attempting to control agent log counter frequency\n" );
		return -1;
	}

	if( piapi_proxy_debug )
		printf( "Successfully controlled agent log counter frequency\n");

	return 0;
}

int
piapi_proxy_mark( void *cntx )
{
	char buf[ PIAPI_BUF_SIZE ] = "";
	unsigned int len;

	if( piapi_proxy_debug )
		printf( "Marking agent log with %s\n", PIAPI_CNTX(cntx)->command );

	len = sprintf( buf, "mark:%s;", PIAPI_CNTX(cntx)->command );

	if( writen( PIAPI_CNTX(cntx)->fd, buf, len ) < 0 ) {
		printf( "Error while attempting to mark agent log\n" );
		return -1;
	}

	if( piapi_proxy_debug )
		printf( "Successfully marked agent log\n");

	return 0;
}

int
piapi_proxy_countfreq( void *cntx )
{
	char buf[ PIAPI_BUF_SIZE ] = "";
	unsigned int len;

	if( piapi_proxy_debug )
		printf( "Controlling agent counter frequency to %u on port %u\n",
			PIAPI_CNTX(cntx)->frequency, PIAPI_CNTX(cntx)->port );

	strcpy( PIAPI_CNTX(cntx)->command, "countfreq" );
	len = sprintf( buf, "%s:%u:%u;", PIAPI_CNTX(cntx)->command,
		PIAPI_CNTX(cntx)->port, PIAPI_CNTX(cntx)->frequency );

	if( writen( PIAPI_CNTX(cntx)->fd, buf, len ) < 0 ) {
		printf( "Error while attempting to control agent counter frequency\n" );
		return -1;
	}

	if( piapi_proxy_debug )
		printf( "Successfully controlled agent counter frequency\n");

	return 0;
}

int
piapi_proxy_init( void *cntx )
{
	if( piapi_proxy_connect( cntx ) )
	{
		printf( "ERROR: unable to start proxy\n" );
		return -1;
	}

	pthread_create(&(PIAPI_CNTX(cntx)->worker), 0x0, (void *)&piapi_proxy_thread, cntx);

	return 0;
}

int
piapi_proxy_destroy( void *cntx )
{
	PIAPI_CNTX(cntx)->worker_run = 0;
	pthread_cancel( PIAPI_CNTX(cntx)->worker ); /* Can't join, could be on blocking read */
	close( PIAPI_CNTX(cntx)->fd );

	return 0;
}
