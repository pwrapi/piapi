#ifndef PIAPI_H
#define PIAPI_H

#include "picommon.h"

int piapi_init( void **cntx, piapi_mode_t mode, piapi_callback_t callback );
int piapi_destroy( void *cntx );

int piapi_collect( void *cntx, piapi_port_t port, unsigned int samples, unsigned int frequency );

int piapi_counter( void *cntx, piapi_port_t port, piapi_sample_t *sample );
int piapi_clear( void *cntx, piapi_port_t port );

#endif
