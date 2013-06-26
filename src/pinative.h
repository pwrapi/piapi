#ifndef PINATIVE_H
#define PINATIVE_H

#include "picommon.h"
#include "pidev.h"

int piapi_native_init( void *cntx );
int piapi_native_destroy( void *cntx );

int piapi_native_collect( void *cntx );
int piapi_native_counter( void *cntx, piapi_port_t port, piapi_sample_t *sample );
int piapi_native_clear( void *cntx, piapi_port_t port );

void piapi_native_thread( void *cntx );

#endif
