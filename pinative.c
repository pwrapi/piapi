#include "pinative.h"
#include "piutil.h"
#include "pidev.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#define MS 1000000.0
#define KS 1000.0

#ifndef PIAPI_NATIVE_DEBUG
static int piapi_native_debug = 0;
#else
static int piapi_native_debug = 1;
#endif

static piapi_counters_t counters;
static pthread_mutex_t piapi_dev_lock;
static int
piapi_dev_collect( piapi_port_t port, piapi_reading_t *reading )
{
    reading_t raw;

#ifdef PIAPI_SPI
    pthread_mutex_lock(&piapi_dev_lock);
    pidev_read(port, &raw);
    pthread_mutex_unlock(&piapi_dev_lock);
#endif

    reading->volts = raw.milivolts/KS;
    reading->amps = raw.miliamps/KS;
    reading->watts = raw.miliwatts/KS;

    return 0;
}

static inline void
piapi_dev_stats( piapi_sample_t *sample, piapi_reading_t *avg,
	piapi_reading_t *min, piapi_reading_t *max, struct timeval *tinit )
{
	struct timeval t;

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
			(t.tv_usec - tinit->tv_usec)/MS;

		sample->energy += sample->raw.watts *
			(t.tv_sec - tinit->tv_sec + (t.tv_usec - tinit->tv_usec)/MS);
	}
}

#ifdef PIAPI_COUNTERS
static unsigned int frequency = PIAPI_SAMPLE_FREQ;

static void
piapi_native_counters( void *arg )
{
	unsigned int frequency = *((unsigned int *)arg);
	unsigned int i;
	struct timeval t0, t1;
	unsigned long tdiff;

	bzero( &counters.sampler, sizeof( piapi_counter_t ) * PIAPI_PORT_ALL );

	if( piapi_native_debug )
		printf( "Counter thread running\n" );

	counters.samplers_run = 1;
	while( counters.samplers_run ) {
		gettimeofday( &t0, 0x0 );
		for( i = PIAPI_PORT_MIN; i <= PIAPI_PORT_MAX; i++ ) {
			unsigned int j = ++(counters.sampler[i].number)%PIAPI_SAMPLE_RING_SIZE;
			counters.sampler[i].sample[j].port = i;
			counters.sampler[i].sample[j].number = j;
			counters.sampler[i].sample[j].total = j;

			if( piapi_dev_collect( i,
				&(counters.sampler[i].sample[j].raw ) ) < 0 ) {
				printf( "Unable to collect reading on port %d", i );
				return;
			}

			piapi_dev_stats( &(counters.sampler[i].sample[j]), &(counters.sampler[i].avg),
				&(counters.sampler[i].min), &(counters.sampler[i].max), &(counters.sampler[i].t) );

			if( !(j % PIAPI_SAMPLE_FREQ) )
				piapi_print( i, &(counters.sampler[i].sample[j]), 0 );

			if( piapi_native_debug )
				piapi_print( i, &(counters.sampler[i].sample[j]), 1 );
		}
		gettimeofday( &t1, 0x0 );
		tdiff = t1.tv_sec - t0.tv_sec +
			(t1.tv_usec - t0.tv_usec)/MS;

		if( tdiff < MS / frequency )
			usleep( MS / frequency - tdiff );
	}

	if( piapi_native_debug )
		printf( "Counter thread exiting\n" );
}
#endif

static void
piapi_native_thread( void *cntx )
{
	piapi_sample_t sample;
	piapi_reading_t min[PIAPI_PORT_ALL], max[PIAPI_PORT_ALL], avg[PIAPI_PORT_ALL];
	struct timeval t[PIAPI_PORT_ALL], t0, t1;
	unsigned long tdiff;

	bzero( &sample, sizeof( piapi_sample_t ) );
	bzero( min, sizeof( piapi_reading_t ) * PIAPI_PORT_ALL );
	bzero( max, sizeof( piapi_reading_t ) * PIAPI_PORT_ALL );
	bzero( avg, sizeof( piapi_reading_t ) * PIAPI_PORT_ALL );
	bzero( t, sizeof( struct timeval ) * PIAPI_PORT_ALL );

	sample.cntx = cntx;
	sample.number = 0;
	sample.total = PIAPI_CNTX(cntx)->samples;

	PIAPI_CNTX(cntx)->worker_run = 1;
	while( PIAPI_CNTX(cntx)->worker_run &&
		(PIAPI_CNTX(cntx)->samples == 0 || sample.number < PIAPI_CNTX(cntx)->samples) ) {
		sample.number++;
		gettimeofday( &t0, 0x0 );
		if( PIAPI_CNTX(cntx)->callback ) {
			unsigned int begin, end;

			begin = PIAPI_CNTX(cntx)->port;
			end = PIAPI_CNTX(cntx)->port;
			if( PIAPI_CNTX(cntx)->port == PIAPI_PORT_ALL ) {
				begin = PIAPI_PORT_MIN;
				end = PIAPI_PORT_MAX;
			} else if( PIAPI_CNTX(cntx)->port == PIAPI_PORT_HALF ) {
				begin = PIAPI_PORT_MIN;
				end = PIAPI_PORT_MID;
			}
			for( sample.port = begin; sample.port <= end; sample.port++ ) {
				if( piapi_dev_collect( sample.port, &(sample.raw) ) < 0 ) {
					printf( "Unable to collect reading on port %d", sample.port);
					return;
				}

				piapi_dev_stats( &sample, &(avg[sample.port]), &(min[sample.port]),
					&(max[sample.port]), &(t[sample.port]) );
				PIAPI_CNTX(cntx)->callback( &sample );
			}
		}
		gettimeofday( &t1, 0x0 );
		tdiff = t1.tv_sec - t0.tv_sec +
			(t1.tv_usec - t0.tv_usec)/MS;

		if( tdiff < MS / PIAPI_CNTX(cntx)->frequency )
			usleep( MS / PIAPI_CNTX(cntx)->frequency - tdiff );
	}
}

int
piapi_native_init( void *cntx )
{
	if( piapi_native_debug )
       		printf( "\nPower native communication\n" );

	pthread_mutex_init(&piapi_dev_lock, NULL);
#ifdef PIAPI_COUNTERS
	if( PIAPI_CNTX(cntx)->mode == PIAPI_MODE_AGENT )
		pthread_create(&counters.samplers, 0x0, (void *)&piapi_native_counters, &frequency);
#endif
	if( piapi_native_debug )
       		printf( "Native counters initialized\n" );

	return 0;
}

int
piapi_native_destroy( void *cntx )
{
	if( piapi_native_debug )
       		printf( "Native counters shutting down\n" );

	PIAPI_CNTX(cntx)->worker_run = 0;
	pthread_join( PIAPI_CNTX(cntx)->worker, NULL);

#ifdef PIAPI_COUNTERS
	if( PIAPI_CNTX(cntx)->mode == PIAPI_MODE_AGENT ) {
		counters.samplers_run = 0;
		pthread_join( counters.samplers, NULL );
	}
#endif
	pthread_mutex_destroy(&piapi_dev_lock);

#ifdef PIAPI_SPI
	pidev_close();
#endif

	if( piapi_native_debug )
       		printf( "Native communication closed\n" );

	return 0;
}

int
piapi_native_collect( void *cntx )
{
	if( piapi_native_debug )
       		printf( "Starting native collection\n" );

	pthread_create(&(PIAPI_CNTX(cntx)->worker), 0x0, (void *)&piapi_native_thread, cntx);
	return 0;
}

int
piapi_native_halt( void *cntx )
{
	if( piapi_native_debug )
       		printf( "Halting native collection\n" );

	PIAPI_CNTX(cntx)->worker_run = 0;
	pthread_join( PIAPI_CNTX(cntx)->worker, NULL);
	return 0;
}

int
piapi_native_counter( void *cntx )
{
	piapi_port_t port = PIAPI_CNTX(cntx)->port;
	unsigned int i, begin, end;
	piapi_sample_t sample;

	if( piapi_native_debug )
       		printf( "Collecting native counter on port %u\n", port );

	begin = port;
	end = port;
	if( port == PIAPI_PORT_ALL ) {
		begin = PIAPI_PORT_MIN;
		end = PIAPI_PORT_MAX;
	} else if( port == PIAPI_PORT_HALF ) {
		begin = PIAPI_PORT_MIN;
		end = PIAPI_PORT_MID;
	}

	for( port = begin; port <= end; port++ ) {
		i = counters.sampler[port].number%PIAPI_SAMPLE_RING_SIZE;
		sample = counters.sampler[port].sample[i];
		sample.cntx = cntx;

		if( piapi_native_debug )
       			printf( "Collecting native counter on port %u\n", port );

		if( PIAPI_CNTX(cntx)->callback ) {
			PIAPI_CNTX(cntx)->callback( &sample );
		}
	}

	return 0;
}

int
piapi_native_reset( void *cntx )
{
	piapi_port_t port = PIAPI_CNTX(cntx)->port;
	unsigned int begin, end;

	if( piapi_native_debug )
       		printf( "Reseting native counter on port %u\n", port );

	begin = port;
	end = port;
	if( port == PIAPI_PORT_ALL ) {
		begin = PIAPI_PORT_MIN;
		end = PIAPI_PORT_MAX;
	} else if( port == PIAPI_PORT_HALF ) {
		begin = PIAPI_PORT_MIN;
		end = PIAPI_PORT_MID;
	}

	for( port = begin; port <= end; port++ ) {
		if( piapi_native_debug )
       			printf( "Reseting native counter on port %u\n", port );
		bzero( &(counters.sampler[port]), sizeof( piapi_counter_t ) );
		counters.sampler[port].generation++;
	}

	return 0;
}

