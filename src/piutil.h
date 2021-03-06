/* 
 * Copyright 2013-2016 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work 
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 */

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

#define PIAPI_BUF_SIZE (1 << 10)
#define PIAPI_SAMPLE_RING_SIZE (1 << 15)

typedef struct piapi_counter {
	piapi_sample_t sample[PIAPI_SAMPLE_RING_SIZE];
	unsigned int generation;
	unsigned int log;
	unsigned int train;

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

void piapi_print_header( FILE *fd );
void piapi_print( FILE *fd, piapi_sample_t *sample, int verbose );

#endif
