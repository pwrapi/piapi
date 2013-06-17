#ifndef PICOMM_H
#define PICOMM_H

#include "piapi.h"

// The well-known powerinsight agent saddr
#define PIAPI_AGNT_SADDR    0x0a361500

// The well-known powerinsight agent port
#define PIAPI_AGNT_PORT     20201

struct picomm_sample {
        unsigned int number;
        unsigned int total;
        unsigned long time_sec;
        unsigned long time_usec;
        float power;
        float energy;
};

typedef void (*picomm_callback_t)( struct picomm_sample *);

int picomm_init( void **cntx, picomm_callback_t callback );

int picomm_destroy( void *cntx );

int picomm_collect( void *cntx, piapi_port_t port, unsigned int samples, unsigned int frequency );

#endif
