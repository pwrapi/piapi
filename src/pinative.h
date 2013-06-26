#ifndef PINATIVE_H
#define PINATIVE_H

#include "picommon.h"
#include "pidev.h"

int piapi_native_collect( piapi_port_t port, piapi_reading_t *reading );

inline void piapi_native_stats( piapi_sample_t *sample, piapi_reading_t *avg,
	piapi_reading_t *min, piapi_reading_t *max, struct timeval *tinit );

int piapi_native_close( void );

void piapi_native_counter( void *cntx, piapi_port_t port, piapi_sample_t *sample );
void piapi_native_clear( void *cntx, piapi_port_t port );

void piapi_native_counters( void *arg );

void piapi_native_thread( void *cntx );

#endif
