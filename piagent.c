#include "piagent.h"
#include "pinative.h"
#include "piutil.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#ifndef PIAPI_AGENT_DEBUG
static int piapi_agent_debug = 0;
#else
static int piapi_agent_debug = 1;
#endif

static int 
piapi_agent_parse( char *buf, unsigned int len, void *cntx )
{
	char *token;

	if( piapi_agent_debug )
		printf( "Parsing agent message %s\n", buf );

	if( (token = strtok( buf, ":" )) == NULL)
		return -1;

	if( !strcmp( token, "collect" ) ) {
		strcpy( PIAPI_CNTX(cntx)->command, token );

		if( (token = strtok( NULL, ":" )) == NULL)
			return -1;
		PIAPI_CNTX(cntx)->port = atoi(token);

		if( (token = strtok( NULL, ":" )) == NULL)
			return -1;
		PIAPI_CNTX(cntx)->samples = atoi(token);

		if( (token = strtok( NULL, ":" )) == NULL)
			return -1;
		PIAPI_CNTX(cntx)->frequency = atoi(token);

		if( piapi_agent_debug ) {
			printf( "Command:   %s\n", PIAPI_CNTX(cntx)->command );
			printf( "Port:      %d\n", PIAPI_CNTX(cntx)->port );
			printf( "Samples:   %d\n", PIAPI_CNTX(cntx)->samples );
			printf( "Frequency: %d\n", PIAPI_CNTX(cntx)->frequency );
		} 

		return 0;
	} else if( !strcmp( token, "counter" ) ) {
		strcpy( PIAPI_CNTX(cntx)->command, token );

		if( (token = strtok( NULL, ":" )) == NULL)
			return -1;
		PIAPI_CNTX(cntx)->port = atoi(token);

		if( piapi_agent_debug ) {
			printf( "Command:   %s\n", PIAPI_CNTX(cntx)->command );
			printf( "Port:      %d\n", PIAPI_CNTX(cntx)->port );
		} 

		return 0;
	} else if( !strcmp( token, "reset" ) ) {
		strcpy( PIAPI_CNTX(cntx)->command, token );

		if( (token = strtok( NULL, ":" )) == NULL)
			return -1;
		PIAPI_CNTX(cntx)->port = atoi(token);

		if( piapi_agent_debug ) {
			printf( "Command:   %s\n", PIAPI_CNTX(cntx)->command );
			printf( "Port:      %d\n", PIAPI_CNTX(cntx)->port );
		} 

		return 0;
	}

	return -1;
}

static void
piapi_agent_callback( piapi_sample_t *sample )
{
	char buf[256] = "";
	unsigned int len;

	len = sprintf( buf, "%u:%u:%lu:%lu:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f",
		sample->number, sample->total, sample->time_sec, sample->time_usec,
		sample->raw.volts, sample->raw.amps, sample->raw.watts,
		sample->avg.volts, sample->avg.amps, sample->avg.watts,
		sample->min.volts, sample->min.amps, sample->min.watts,
		sample->max.volts, sample->max.amps, sample->max.watts,
		sample->time_total, sample->energy );

	if( piapi_agent_debug )
		printf( "Sending sample (%d) %s\n", len, buf );

	if( sample->cntx ) {
		if( writen( PIAPI_CNTX(sample->cntx)->cfd, buf, len ) < 0 ) {
			PIAPI_CNTX(sample->cntx)->sample_run = 0;
			pthread_join( PIAPI_CNTX(sample->cntx)->sample, 0x0 );
			printf( "Connection closed" );
			return;
		}
	} else {
		printf( "Missing sample context\n" );
		return;
	}
}

static int
piapi_agent_listen( void *cntx )
{
	struct sockaddr_in addr;
	ssize_t rc;

	if( piapi_agent_debug )
		printf( "Establishing agent listener\n" );

        PIAPI_CNTX(cntx)->fd = socket( PF_INET, SOCK_STREAM, 0 );
        if( PIAPI_CNTX(cntx)->fd < 0 ) {
                printf( "ERROR: socket() failed! rc=%d\n", PIAPI_CNTX(cntx)->fd );
                return -1;
        }

	bzero((void *)&addr, sizeof(addr));
        addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons( PIAPI_CNTX(cntx)->sa_port );

	rc = bind( PIAPI_CNTX(cntx)->fd, (struct sockaddr *) &addr, sizeof(addr) );
	if( rc < 0 ) {
		printf( "ERROR: bind() failed!\n" );
		perror( "bind" );
		return -1;
	}

	rc = listen( PIAPI_CNTX(cntx)->fd, 5 );
	if( rc < 0 ) {
		printf( "ERROR: listen() failed!\n" );
		perror( "listen" );
		return -1;
	}

	if( piapi_agent_debug )
		printf( "Agent is listening on port %d\n", PIAPI_CNTX(cntx)->sa_port );

	return 0;
}

static void
piapi_agent_counter( void *cntx )
{
	piapi_native_counter( cntx );
}

static void
piapi_agent_thread( void *cntx )
{
	struct sockaddr_in addr;
	socklen_t socklen = sizeof(addr);

	int max_fd = PIAPI_CNTX(cntx)->fd;
	fd_set fds;
	FD_ZERO( &fds );
	FD_SET( PIAPI_CNTX(cntx)->fd, &fds );

	PIAPI_CNTX(cntx)->worker_run = 1;
	while( PIAPI_CNTX(cntx)->worker_run ) {
		fd_set read_fds = fds;
		int rc = select( max_fd+1, &read_fds, 0, 0, 0 );
		if( rc < 0 ) {
			printf( "ERROR: select() failed! rc=%d\n", rc );
			return;
		}

		if( FD_ISSET( PIAPI_CNTX(cntx)->fd, &read_fds ) ) {
			if( piapi_agent_debug )
				printf( "Proxy establishing connection\n");

			int new_fd = accept( PIAPI_CNTX(cntx)->fd, (struct sockaddr *) &addr, &socklen );
			if( piapi_agent_debug )
				printf( "Proxy IP address is %d.%d.%d.%d\n",
					*((char *)(&addr.sin_addr.s_addr)+0),
					*((char *)(&addr.sin_addr.s_addr)+1),
					*((char *)(&addr.sin_addr.s_addr)+2),
					*((char *)(&addr.sin_addr.s_addr)+3) );

			FD_SET( new_fd, &fds );
			if( new_fd > max_fd )
				max_fd = new_fd;
		}

		int fd;
		for( fd=0 ; fd<=max_fd ; fd++ ) {
			if( fd == PIAPI_CNTX(cntx)->fd || !FD_ISSET( fd, &read_fds ) )
				continue;

			char buf[ 256 ];
			ssize_t rc = read( fd, buf, sizeof(buf)-1 );
			if( rc <= 0 ) {
				FD_CLR( fd, &fds );
				continue;
			}

			if( rc == 0 )
				continue;

			buf[rc] = '\0';
			while( rc > 0 ) {
				if( !isspace( buf[rc-1] ) )
					break;
				buf[--rc] = '\0';
			}

			PIAPI_CNTX(cntx)->cfd = fd;
			piapi_agent_parse( buf, rc, cntx );

			if( !strcmp( PIAPI_CNTX(cntx)->command, "collect" ) ) {
				piapi_native_collect( cntx );
			} else if( !strcmp( PIAPI_CNTX(cntx)->command, "counter" ) ) {
				piapi_agent_counter( cntx );
			} else if( !strcmp( PIAPI_CNTX(cntx)->command, "reset" ) ) {
				piapi_native_reset( cntx );
			}
		}
	}
}

int
piapi_agent_init( void *cntx )
{
	if( piapi_agent_debug )
       		printf( "\nPower native communication\n" );

	if( piapi_agent_listen( cntx ) )
	{
		printf( "ERROR: unable to start agent\n" );
		return -1;
	}

	PIAPI_CNTX(cntx)->callback = piapi_agent_callback;
	pthread_create(&(PIAPI_CNTX(cntx)->worker), 0x0, (void *)&piapi_agent_thread, cntx);

	piapi_native_init( cntx );

	if( piapi_agent_debug )
       		printf( "Agent listener established\n" );

	return 0;
}

int
piapi_agent_destroy( void *cntx )
{
	PIAPI_CNTX(cntx)->worker_run = 0;
	pthread_join( PIAPI_CNTX(cntx)->worker, NULL );
	close( PIAPI_CNTX(cntx)->fd );

	piapi_native_destroy( cntx );

	return 0;
}

