#ifndef PIUTIL_H
#define PIUTIL_H

#include "picommon.h"

#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>

/* The well-known powerinsight agent saddr */
#define PIAPI_AGNT_SADDR    0x0a360003

/* The well-known powerinsight agent port */
#define PIAPI_AGNT_PORT     20201

#define SAMPLE_FREQ 10
#define SAMPLE_RING_SIZE (1 << 15)

typedef struct piapi_counter {
	piapi_sample_t sample[SAMPLE_RING_SIZE];

	unsigned int number;
	piapi_reading_t min, max, avg;
	struct timeval t;
} piapi_counter_t;

typedef struct piapi_counters {
	piapi_counter_t sampler[PIAPI_PORT_ALL];

	pthread_t samplers;
	int samplers_run;
} piapi_counters_t;

struct piapi_context {
	int fd, cfd;
	piapi_mode_t mode;
	unsigned int sa_addr;
	unsigned short sa_port;

	char command[40];
	piapi_port_t port;
	unsigned int samples;
	unsigned int frequency;

	piapi_callback_t callback;
	pthread_t worker;
	int worker_run;
};

#define PIAPI_CNTX(X) ((struct piapi_context *)(X))

ssize_t writen(int fd, const void *vptr, size_t n);

void piapi_print( piapi_port_t port, struct piapi_sample *sample );

#endif
