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

static int piapi_native_debug = 0;
static unsigned int frequency = SAMPLE_FREQ;

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
			(t.tv_usec - tinit->tv_usec)/1000000.0;

		sample->energy += sample->raw.watts *
			(t.tv_sec - tinit->tv_sec + (t.tv_usec - tinit->tv_usec)/1000000.0);
	}
}

int
piapi_native_collect( piapi_port_t port, piapi_reading_t *reading )
{
    reading_t raw;
    pidev_read(port, &raw);

    reading->volts = raw.milivolts/1000.0;
    reading->amps = raw.miliamps/1000.0;
    reading->watts = raw.miliwatts/1000.0;

    return 0;
}

inline void
piapi_native_stats( piapi_sample_t *sample, piapi_reading_t *avg,
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
			(t.tv_usec - tinit->tv_usec)/1000000.0;

		sample->energy += sample->raw.watts *
			(t.tv_sec - tinit->tv_sec + (t.tv_usec - tinit->tv_usec)/1000000.0);
	}
}

int
piapi_native_close( void )
{
	pidev_close();

	return 0;
}

void
piapi_native_counter( void *cntx, piapi_port_t port, piapi_sample_t *sample )
{
	unsigned int i = counters.sampler[port].number%SAMPLE_RING_SIZE;
	*sample = counters.sampler[port].sample[i];
	sample->cntx = cntx;
}

void
piapi_native_clear( void *cntx, piapi_port_t port )
{
	bzero( &(counters.sampler[port]), sizeof( piapi_counter_t ) );
}

void
piapi_native_counters( void *arg )
{
	unsigned int frequency = *((unsigned int *)arg);

	unsigned int i;

	bzero( &counters.sampler, sizeof( piapi_counter_t ) * PIAPI_PORT_MAX );

	if( piapi_native_debug )
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

			if( piapi_native_debug )
				piapi_print( i, &(counters.sampler[i].sample[j]) );
		}
		usleep( 1000000.0 / frequency );
	}

	if( piapi_native_debug )
		printf( "Counter thread exiting\n" );
}

void
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

