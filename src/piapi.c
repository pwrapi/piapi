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

static int piapi_debug = 0;

#define SAMPLE_FREQ 10
#define SAMPLE_RING_SIZE (1 << 15)

typedef struct piapi_counter {
	piapi_sample_t sample[SAMPLE_RING_SIZE];

	unsigned int number;
	piapi_reading_t min, max, avg;
	struct timeval t;
} piapi_counter_t;

typedef struct piapi_counters {
	piapi_counter_t sampler[PIAPI_PORT_MAX];

	pthread_t samplers;
	int samplers_run;
} piapi_counters_t;

static piapi_counters_t counters;
static unsigned int frequency = SAMPLE_FREQ;

struct piapi_context {
	int fd, cfd;
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

static void
piapi_print( piapi_port_t port, struct piapi_sample *sample )
{
        printf( "Sample on port %d:\n", port);
        printf( "\tsample       - %u of %u\n", sample->number, sample->total );
        printf( "\ttime         - %f\n", sample->time_sec+sample->time_usec/1000000.0 );
        printf( "\tvolts        - %f\n", sample->raw.volts );
        printf( "\tamps         - %f\n", sample->raw.amps );
        printf( "\twatts        - %f\n", sample->raw.watts );

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

static inline void
piapi_dev_stats( piapi_sample_t *sample, piapi_reading_t *avg,
	piapi_reading_t *min, piapi_reading_t *max, struct timeval *tinit )
{
	struct timeval t, tprev;

	tprev.tv_sec = sample->time_sec;
	tprev.tv_usec = sample->time_usec;

	gettimeofday( &t, 0x0 );
	sample->time_sec = t.tv_sec;
	sample->time_usec = t.tv_usec;

	if( min->volts > sample->raw.volts || !min->volts) min->volts = sample->raw.volts;
	if( min->amps > sample->raw.amps || !min->amps ) min->amps = sample->raw.amps;
	if( min->watts > sample->raw.watts || !min->watts ) min->watts = sample->raw.watts;

	if( max->volts < sample->raw.volts ) max->volts = sample->raw.volts;
	if( max->amps < sample->raw.amps ) max->amps = sample->raw.amps;
	if( max->watts < sample->raw.watts ) max->watts = sample->raw.watts;

	avg->volts += sample->raw.volts;
	avg->amps += sample->raw.amps;
	avg->watts += sample->raw.watts;

	sample->min.volts = min->volts;
	sample->min.amps = min->amps;
	sample->min.watts = min->watts;

	sample->max.volts = max->volts;
	sample->max.amps = max->amps;
	sample->max.watts = max->watts;

	sample->avg.volts = avg->volts / sample->number;
	sample->avg.amps = avg->amps / sample->number;
	sample->avg.watts = avg->watts / sample->number;

	if( !tinit->tv_sec ) {
		tinit->tv_sec = sample->time_sec;
		tinit->tv_usec = sample->time_usec;

		sample->time_total = 0;
		sample->energy = sample->raw.watts;
	} else {
		sample->time_total = t.tv_sec - tinit->tv_sec +
			(t.tv_usec - tinit->tv_usec)/1000000.0;

		sample->energy += sample->raw.watts *
			(t.tv_sec - tprev.tv_sec + (t.tv_usec - tprev.tv_usec)/1000000.0);
	}
}

static int
piapi_dev_close( void )
{
    pidev_close();

    return 0;
}

static void
piapi_counters_thread( void *arg )
{
	unsigned int frequency = *((unsigned int *)arg);

	unsigned int i;

	bzero( &counters.sampler, sizeof( piapi_counter_t ) * PIAPI_PORT_MAX );

	if( piapi_debug )
		printf( "Counter thread running\n" );

	counters.samplers_run = 1;
	while( counters.samplers_run ) {
		for( i = PIAPI_PORT_MIN; i < PIAPI_PORT_MAX; i++ ) {
			unsigned int j = ++(counters.sampler[i].number)%SAMPLE_RING_SIZE;
			counters.sampler[i].sample[j].number = j;
			counters.sampler[i].sample[j].total = j;

			if( piapi_dev_collect( i,
				&(counters.sampler[i].sample[j].raw ) ) < 0 ) {
				printf( "Unable to collect reading on port %d", i );
				return;
			}

			piapi_dev_stats( &(counters.sampler[i].sample[j]), &(counters.sampler[i].avg),
				&(counters.sampler[i].min), &(counters.sampler[i].max), &(counters.sampler[i].t) );

			if( piapi_debug )
				piapi_print( i, &(counters.sampler[i].sample[j]) );
		}
		usleep( 1000000.0 / frequency );
	}

	if( piapi_debug )
		printf( "Counter thread exiting\n" );
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

	return 0;
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

	if( piapi_debug )
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

		if( piapi_debug ) {
			printf( "Command:   %s\n", PIAPI_CNTX(cntx)->command );
			printf( "Port:      %d\n", PIAPI_CNTX(cntx)->port );
			printf( "Samples:   %d\n", PIAPI_CNTX(cntx)->samples );
			printf( "Frequency: %d\n", PIAPI_CNTX(cntx)->frequency );
		} 

		return 0;
	}

	return -1;
}

static int 
piapi_proxy_parse( char *buf, unsigned int len, piapi_sample_t *sample )
{
	char *token;

	if( piapi_debug )
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

	if( piapi_debug ) {
		printf( "\tsample     - %u of %u\n", sample->number, sample->total );
		printf( "\ttime       - %f\n", sample->time_sec+sample->time_usec/1000000.0 );
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
piapi_agent_callback( piapi_sample_t *sample )
{
	char buf[256] = "";
	unsigned int len;

	len = sprintf(buf, "%u:%u:%lu:%lu:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f:%f",
		sample->number, sample->total, sample->time_sec, sample->time_usec,
		sample->raw.volts, sample->raw.amps, sample->raw.watts,
		sample->avg.volts, sample->avg.amps, sample->avg.watts,
		sample->min.volts, sample->min.amps, sample->min.watts,
		sample->max.volts, sample->max.amps, sample->max.watts,
		sample->time_total, sample->energy);

	if( piapi_debug )
		printf( "Sending sample (%d) %s\n", len, buf);

	writen( PIAPI_CNTX(sample->cntx)->cfd, buf, len );
}

static void
piapi_native_thread( void *cntx )
{
	piapi_sample_t sample;
	piapi_reading_t min, max, avg;
	struct timeval t = { 0, 0 };

	sample.cntx = cntx;
	sample.number = 0;
	sample.total = PIAPI_CNTX(cntx)->samples;

	PIAPI_CNTX(cntx)->worker_run = 1;
	while( PIAPI_CNTX(cntx)->worker_run &&
		(PIAPI_CNTX(cntx)->samples == 0 || sample.number < PIAPI_CNTX(cntx)->samples) ) {
		sample.number++;
		if( PIAPI_CNTX(cntx)->callback ) {
			if( piapi_dev_collect( PIAPI_CNTX(cntx)->port, &sample.raw ) < 0 ) {
				printf( "Unable to collect reading on port %d", PIAPI_CNTX(cntx)->port);
				return;
			}

			piapi_dev_stats( &sample, &avg, &min, &max, &t );
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
	piapi_sample_t sample;
	char buf[ 256 ];
	ssize_t rc;

	sample.cntx = cntx;

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
				if( piapi_debug )
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
			if( !strcmp( PIAPI_CNTX(cntx)->command, "collect" ) ) {
				PIAPI_CNTX(cntx)->cfd = fd;
				pthread_create(&(PIAPI_CNTX(cntx)->worker), 0x0, (void *)&piapi_native_thread, cntx);
			}
		}

	}
}

int
piapi_init( void **cntx, piapi_mode_t mode, piapi_callback_t callback )
{
	*cntx = malloc( sizeof(struct piapi_context) );
	bzero( *cntx, sizeof(struct piapi_context) );

	PIAPI_CNTX(*cntx)->mode = mode;

	switch( PIAPI_CNTX(*cntx)->mode ) {
		case PIAPI_MODE_NATIVE:
			if( piapi_debug )
        			printf( "\nPower Communication (Native)\n" );

			PIAPI_CNTX(*cntx)->callback = callback;
			pthread_create(&counters.samplers, 0x0, (void *)&piapi_counters_thread, &frequency);
			break;

		case PIAPI_MODE_PROXY:
			if( piapi_debug )
        			printf( "\nPower Communication (Proxy <=> Agent)\n" );

			if( piapi_agent_connect( *cntx ) )
			{
				printf( "ERROR: unable to start proxy\n" );
				return -1;
			}

			PIAPI_CNTX(*cntx)->callback = callback;
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

			PIAPI_CNTX(*cntx)->callback = piapi_agent_callback;
			pthread_create(&counters.samplers, 0x0, (void *)&piapi_counters_thread, &frequency);

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
			counters.samplers_run = 0;
			piapi_dev_close();
			break;

		case PIAPI_MODE_PROXY:
			close( PIAPI_CNTX(cntx)->fd );
			break;

		case PIAPI_MODE_AGENT:
			counters.samplers_run = 0;
			close( PIAPI_CNTX(cntx)->fd );
			piapi_dev_close();

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

			if( piapi_debug )
				printf("Stopping native collect\n");
			break;

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
	unsigned int i = counters.sampler[port].number%SAMPLE_RING_SIZE;

	*sample = counters.sampler[port].sample[i];

	return 0;
}

int
piapi_clear( void *cntx, piapi_port_t port )
{
	bzero( &(counters.sampler[port]), sizeof( piapi_counter_t ) );
	return 0;
}
