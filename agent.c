#include "piapi.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

static int piapi_sampling = 0;

static void
signal_handler(int sig)
{
	if( sig == SIGINT )
		piapi_sampling = 0;		
	sleep(2);
	exit(0);
}

int
main(int argc, char *argv[])
{
	unsigned int saddr = 0,
		sport = 0;
	int opt;
	char *token;
	void *cntx;

	while( (opt=getopt( argc, argv, "a:p:" )) != -1 ) {
		switch( opt ) {
			case 'a':
				token = strtok( optarg, "." );
				saddr = atoi(token) << 24;
				token = strtok( NULL, "." );
				saddr |= ( atoi(token) << 16 );
				token = strtok( NULL, "." );
				saddr |= ( atoi(token) << 8 );
				token = strtok( NULL, "." );
				saddr |= atoi(token);
				break;
			case 'p':
				sport = atoi(optarg);
				break;
			case '?':
				printf( "Usage: %s [-a sa_addr] [-p sa_port]\n", argv[0] );
				exit( -1 );
			default:
				abort( );
		} 
	}

	signal( SIGINT, signal_handler );
	piapi_init( &cntx, PIAPI_MODE_AGENT, 0x0, saddr, sport ); 

	piapi_sampling = 1;
	while( piapi_sampling );

	piapi_destroy( &cntx );

	return 0;
}

