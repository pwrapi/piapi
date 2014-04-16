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
piapi_print_header( void )
{
	printf( "# number total time_sec time_usec port raw.volts raw.amps raw.watts"
		"avg.volts avg.amps avg.watts min.volts min.amps min.watts"
		"max.volts max.amps max.watts time_total energy" );
}

void
piapi_print( piapi_sample_t *sample, int verbose )
{
	if( !verbose ) {
		printf( "%u %u %lu %lu %u %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
			sample->number, sample->total, sample->time_sec, sample->time_usec,
			sample->port, sample->raw.volts, sample->raw.amps, sample->raw.watts,
			sample->avg.volts, sample->avg.amps, sample->avg.watts,
			sample->min.volts, sample->min.amps, sample->min.watts,
			sample->max.volts, sample->max.amps, sample->max.watts,
			sample->time_total, sample->energy );
	} else {
	        printf( "Sample on port %d:\n", sample->port);
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
}

