/* getRawPower9.2.c    (version 6.2)
 * For use with Carrier Board 10016423 Rev E8 (= Rev A)
 */

#include "pidev.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>
#include <strings.h>

static int timeCollect = 0;

/***********************************************************/
static void printUsage(const char *prog)
{
	puts("");
	printf("Usage: %s [po po ... ]\n", prog);
	puts("    where po == port number to be queried\n"
	     "    po has range [1..7]\n"
	     "    A port is the connector cabled to a sensor board.");
	printf("    Example: %s 1 2 3 4 5 6 7\n\n", prog);
	exit(1);
}

/***********************************************************/
int main(int argc, char *argv[])
{
    int i, portNumber ;
    struct timeval start, now ;
    reading_t sample;

    bzero( &sample, sizeof( reading_t ) );
    if(argc < 2) {
        printUsage(argv[0]);
    }

    // Starting time
    if( timeCollect ) {
        gettimeofday( &start, NULL );
    } else {
      printf("%-4s %5s %5s %7s %7s %7s\n",
          "Pt#", "A", "V", "mA", "mV", "mW"); 
    }

    pidev_open();

    // MAIN EXEC LOOP
    for(i=0; i<(argc-1); i++) 
    { 
        portNumber = atoi(argv[i+1]);
        if((portNumber < 0) || (portNumber > MAX_PORTNUM)) 
            { printUsage(argv[0]); }

        // Collect raw readings and calculate power
        pidev_read(portNumber, &sample);

        // What time is it now?
        if( timeCollect ) {
            gettimeofday( &now, NULL );
        }

        // Print results
        if( timeCollect ) {
            printf("%ld %ld.%06ld %-4d %7f %7f %7f\n", 
                start.tv_sec, now.tv_sec - start.tv_sec, (long int)(now.tv_usec),
                portNumber, sample.amp,sample.volt,sample.watt );
        } else {
            printf("%-4d %7f %7f %7f\n", 
                portNumber, sample.amp,sample.volt,sample.watt );
        }
    }  // end for() loop 

    pidev_close();

    return (0);
}   // end main()

