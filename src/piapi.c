#include "piapi.h"
#include "pidev.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

static int piapi_debug = 1;

struct piapi_context {
	int fd;
	piapi_mode_t mode;

	char command[40];
	piapi_port_t port;
	unsigned int samples;
	unsigned int frequency;

	piapi_callback_t callback;
	pthread_t worker;
	int worker_run;
};

#define PIAPI_CNTX(X) ((struct piapi_context *)(X))

/* writen() is from "UNIX Network Programming" by W. Richard Stevents */
static ssize_t
writen(int fd, const void *vptr, size_t n)
{
        size_t nleft;
        ssize_t nwritten;
        const char *ptr;

        ptr = vptr;
        nleft = n;
        while( nleft > 0 ) {
                if( (nwritten = write(fd, ptr, nleft)) <= 0 ) {
                        if (errno == EINTR) {
                                nwritten = 0;  /* and call write() again */
                        } else {
                                perror( "write" );
                                return -1;     /* error */
                        }
                }
                nleft -= nwritten;
                ptr   += nwritten;
        }

        return n;
}

static int
piapi_dev_collect( piapi_port_t port, piapi_reading_t *reading )
{
    reading_t raw;
    pidev_read(port, &raw);

    reading->volts = raw.milivolts/1000.0;
    reading->amps = raw.miliamps/1000.0;
    reading->watts = raw.miliwatts/1000.0;

    return 0;
}

static int
piapi_dev_close( void )
{
    pidev_close();

    return 0;
}

static int
piapi_agent_listen( void *cntx )
{
	struct sockaddr_in addr;
	ssize_t rc;

	if( piapi_debug )
		printf( "Establishing agent listener\n" );

        PIAPI_CNTX(cntx)->fd = socket( PF_INET, SOCK_STREAM, 0 );
        if( PIAPI_CNTX(cntx)->fd < 0 ) {
                printf( "ERROR: socket() failed! rc=%d\n", PIAPI_CNTX(cntx)->fd );
                return -1;
        }

	bzero((void *)&addr, sizeof(addr));
        addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons( PIAPI_AGNT_PORT );

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

	if( piapi_debug )
		printf( "Agent is listening on port %d\n", PIAPI_AGNT_PORT );
}

static int
piapi_agent_connect( void *cntx )
{
	struct sockaddr_in addr;
	ssize_t rc;

	if( piapi_debug )
		printf( "Establishing agent connection\n" );

        PIAPI_CNTX(cntx)->fd = socket( PF_INET, SOCK_STREAM, 0 );
        if( PIAPI_CNTX(cntx)->fd < 0 ) {
                printf( "ERROR: socket() failed! rc=%d\n", PIAPI_CNTX(cntx)->fd );
                return -1;
        }

	bzero((void *)&addr, sizeof(addr));
        addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl( PIAPI_AGNT_SADDR );
	addr.sin_port = htons( PIAPI_AGNT_PORT );

	rc = connect( PIAPI_CNTX(cntx)->fd, (struct sockaddr *) &addr, sizeof(addr) );
	if( rc < 0 ) {
		printf( "ERROR: connect() failed!\n" );
		perror( "CONNECT" );
		return -1;
	}

	if( piapi_debug )
		printf( "Agent IP address is %d.%d.%d.%d\n",
			*((char *)(&addr.sin_addr.s_addr)+0),
			*((char *)(&addr.sin_addr.s_addr)+1),
			*((char *)(&addr.sin_addr.s_addr)+2),
			*((char *)(&addr.sin_addr.s_addr)+3) );

	if( piapi_debug )
        	printf( "Connected to agent port %d\n", PIAPI_AGNT_PORT );

	return 0;
}

static int 
piapi_agent_parse( char *buf, unsigned int len, void *cntx )
{
	char *token;

	if( (token = strtok( buf, ":" )) == NULL)
		return -1;

	if( !strcmp( token, "collect" ) ) {
		strcpy( PIAPI_CNTX(cntx)->command, token );
		if( (token = strtok( buf, ":" )) == NULL)
			return -1;
		PIAPI_CNTX(cntx)->samples = atoi(token);

		if( (token = strtok( buf, ":" )) == NULL)
			return -1;
		PIAPI_CNTX(cntx)->frequency = atoi(token);

		return 0;
	}

	return -1;
}

static int 
piapi_proxy_parse( char *buf, unsigned int len, piapi_sample_t *sample )
{
	char *token;

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
	sample->raw.volts = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->raw.amps = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->raw.watts = atof(token);

	if( (token = strtok( NULL, ":" )) == NULL)
		return -1;
	sample->energy = atof(token);

	return 0;
}

static void
piapi_native_collect( void *cntx )
{
	struct timeval t;
	piapi_sample_t sample;

	sample.number = 0;
	sample.total = PIAPI_CNTX(cntx)->samples;

	PIAPI_CNTX(cntx)->worker_run = 1;
	while( PIAPI_CNTX(cntx)->worker_run && sample.number < PIAPI_CNTX(cntx)->samples ) {
		sample.number++;
		if( PIAPI_CNTX(cntx)->callback ) {
			if( piapi_dev_collect( PIAPI_CNTX(cntx)->port, &sample.raw ) < 0 ) {
				printf( "Unable to collect reading on port %d", PIAPI_CNTX(cntx)->port);
				return;
			}
			gettimeofday( &t, 0x0 );
			sample.time_sec = t.tv_sec;
			sample.time_usec = t.tv_usec;
			PIAPI_CNTX(cntx)->callback( &sample );
		}
		usleep( 1000000.0 / PIAPI_CNTX(cntx)->frequency );
	}
}

static int
piapi_proxy_collect( void *cntx )
{
	char buf[ 256 ] = "";
	unsigned int len;

	if( piapi_debug )
		printf( "Setting agent to collect on sensor port %u\n", PIAPI_CNTX(cntx)->port);

	strcpy( PIAPI_CNTX(cntx)->command, "collect" );
	len = sprintf( buf, "%s:%u:%u:%u", PIAPI_CNTX(cntx)->command,
		PIAPI_CNTX(cntx)->port, PIAPI_CNTX(cntx)->samples, PIAPI_CNTX(cntx)->frequency );

	writen( PIAPI_CNTX(cntx)->fd, buf, len );

	if( piapi_debug )
		printf( "Successfully started collect\n");

	return 0;
}

static void
piapi_proxy_thread( void *cntx )
{
	struct sockaddr_in addr;
	socklen_t socklen = sizeof(addr);

	piapi_sample_t sample;
	char buf[ 256 ];
	ssize_t rc;

	PIAPI_CNTX(cntx)->worker_run = 1;
	while( PIAPI_CNTX(cntx)->worker_run ) {
		if( piapi_debug )
			printf( "%d: attempting read\n", PIAPI_CNTX(cntx)->fd);

		rc = read( PIAPI_CNTX(cntx)->fd, buf, sizeof(buf)-1 );
		if( rc <= 0 ) {
			if( piapi_debug )
				printf( "%d: closed connection rc=%zd\n", PIAPI_CNTX(cntx)->fd, rc );
			continue;
		}

		if( piapi_debug )
			printf( "%d: checking read length\n", PIAPI_CNTX(cntx)->fd);

		if( rc == 0 )
			continue;

		buf[rc] = '\0';
		while( rc > 0 ) {
			if( !isspace( buf[rc-1] ) )
				break;
			buf[--rc] = '\0';
		}

		if( piapi_debug )
			printf( "%d: read %zd bytes: '%s'\n", PIAPI_CNTX(cntx)->fd, rc, buf);

		if( PIAPI_CNTX(cntx)->callback ) {
			piapi_proxy_parse( buf, rc, &sample );
			PIAPI_CNTX(cntx)->callback( &sample );
		}
	}
}

static void
piapi_agent_collect( void *cntx )
{
	char buf[256] = "";
	unsigned int len;

	struct timeval t;
	piapi_sample_t sample;

	sample.number = 0;
	sample.total = PIAPI_CNTX(cntx)->samples;

	while( PIAPI_CNTX(cntx)->worker_run && sample.number < PIAPI_CNTX(cntx)->samples ) {
		sample.number++;
		if( PIAPI_CNTX(cntx)->fd ) {
			if( piapi_dev_collect( PIAPI_CNTX(cntx)->port, &sample.raw ) < 0 ) {
				printf( "Unable to collect reading on port %d", PIAPI_CNTX(cntx)->port);
				return;
			}
			gettimeofday( &t, 0x0 );
			sample.time_sec = t.tv_sec;
			sample.time_usec = t.tv_usec;

			len = sprintf(buf, "%u:%u:%lu:%lu:%f:%f:%f:%f",
				sample.number, sample.total, sample.time_sec, sample.time_usec,
				sample.raw.volts, sample.raw.amps, sample.raw.watts, sample.energy);

			writen( PIAPI_CNTX(cntx)->fd, buf, len );

			if( PIAPI_CNTX(cntx)->callback )
				PIAPI_CNTX(cntx)->callback( &sample );
		}
		usleep( 1000000.0 / PIAPI_CNTX(cntx)->frequency );
	}
}

static void
piapi_agent_thread( void *cntx )
{
	pthread_t collect;

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
			if( piapi_debug )
				printf( "Proxy establishing connection\n");

			int new_fd = accept( PIAPI_CNTX(cntx)->fd, (struct sockaddr *) &addr, &socklen );
			if( piapi_debug )
				printf( "Agent IP address is %d.%d.%d.%d\n",
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
				printf( "%d: closed connection rc=%zd\n", fd, rc );
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

			if( piapi_debug )
				printf( "%d: read %zd bytes: '%s'\n", fd, rc, buf);

			piapi_agent_parse( buf, rc, cntx );
			if( strcmp( PIAPI_CNTX(cntx)->command, "collect" ) )
				pthread_create( &collect, 0x0, (void *)&piapi_agent_collect, cntx );	
		}

	}
}

int
piapi_init( void **cntx, piapi_mode_t mode, piapi_callback_t callback )
{

	*cntx = malloc( sizeof(struct piapi_context) );
	bzero( *cntx, sizeof(struct piapi_context) );

	PIAPI_CNTX(*cntx)->callback = callback;
	PIAPI_CNTX(*cntx)->mode = mode;

	switch( PIAPI_CNTX(*cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
			if( piapi_debug )
        			printf( "\nPower Communication (Native)\n" );
			break;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
        			printf( "\nPower Communication (Proxy <=> Agent)\n" );

			if( piapi_agent_connect( *cntx ) )
			{
				printf( "ERROR: unable to start proxy\n" );
				return -1;
			}

			pthread_create(&(PIAPI_CNTX(*cntx)->worker), 0x0, (void *)&piapi_proxy_thread, *cntx);

			if( piapi_debug )
       				printf( "Agent connection established\n" );
			break;

		case PIAPI_MODE_AGENT:
			if( piapi_debug )
        			printf( "\nPower Communication (Agent <=> Proxy)\n" );

			if( piapi_agent_listen( *cntx ) )
			{
				printf( "ERROR: unable to start agent\n" );
				return -1;
			}

			pthread_create(&(PIAPI_CNTX(*cntx)->worker), 0x0, (void *)&piapi_agent_thread, *cntx);

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
			piapi_dev_close();
			break;

		case PIAPI_MODE_PROXY:
		case PIAPI_MODE_AGENT:
			close( PIAPI_CNTX(cntx)->fd );
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
			pthread_create(&(PIAPI_CNTX(cntx)->worker), 0x0, (void *)&piapi_native_collect, cntx);
			break;

		case PIAPI_MODE_PROXY:
		case PIAPI_MODE_AGENT:
			piapi_proxy_collect( cntx );
			break;

		default:
			break;
	}
}
