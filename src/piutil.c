/* 
 * Copyright 2013-2015 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work 
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 */

#include "piutil.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

ssize_t
writen(int fd, const void *vptr, size_t n)
{
        size_t nleft;
        ssize_t nwritten;
        const char *ptr;

	if( fd < 0 || !vptr) {
		printf( "File descriptor (%d) or vptr (%p) is invalid\n", fd, vptr );
		return 0;
	}

        ptr = vptr;
        nleft = n;
        while( nleft > 0 ) {
                if( (nwritten = write(fd, ptr, nleft)) <= 0 ) {
                        if( errno == EINTR ) {
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

void
piapi_print_header( FILE *fd )
{
	fprintf( fd, "# number total time_sec time_usec port raw.volts raw.amps raw.watts "
		"avg.volts avg.amps avg.watts min.volts min.amps min.watts "
		"max.volts max.amps max.watts time_total energy\n" );
}

void
piapi_print( FILE *fd, piapi_sample_t *sample, int verbose )
{
	if( !verbose ) {
		fprintf( fd, "%u %u %lu %lu %u %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
			sample->number, sample->total, sample->time_sec, sample->time_usec,
			sample->port, sample->raw.volts, sample->raw.amps, sample->raw.watts,
			sample->avg.volts, sample->avg.amps, sample->avg.watts,
			sample->min.volts, sample->min.amps, sample->min.watts,
			sample->max.volts, sample->max.amps, sample->max.watts,
			sample->time_total, sample->energy );
	} else {
	        fprintf( fd, "Sample on port %d:\n", sample->port);
        	fprintf( fd, "\tsample       - %u of %u\n", sample->number, sample->total );
	        fprintf( fd, "\ttime         - %f\n", sample->time_sec+sample->time_usec/1000000.0 );
        	fprintf( fd, "\tvolts        - %f\n", sample->raw.volts );
        	fprintf( fd, "\tamps         - %f\n", sample->raw.amps );
        	fprintf( fd, "\twatts        - %f\n", sample->raw.watts );

        	fprintf( fd, "\tavg volts    - %f\n", sample->avg.volts );
       		fprintf( fd, "\tavg amps     - %f\n", sample->avg.amps );
  		fprintf( fd, "\tavg watts    - %f\n", sample->avg.watts );

        	fprintf( fd, "\tmin volts    - %f\n", sample->min.volts );
       		fprintf( fd, "\tmin amps     - %f\n", sample->min.amps );
  		fprintf( fd, "\tmin watts    - %f\n", sample->min.watts );

        	fprintf( fd, "\tmax volts    - %f\n", sample->max.volts );
       		fprintf( fd, "\tmax amps     - %f\n", sample->max.amps );
  		fprintf( fd, "\tmax watts    - %f\n", sample->max.watts );

        	fprintf( fd, "\ttotal time   - %f\n", sample->time_total );
        	fprintf( fd, "\ttotal energy - %f\n", sample->energy );
	}
}

