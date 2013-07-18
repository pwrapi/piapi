#include "piapi.h"

int main(int argc, char *argv[])
{
	void *cntx;

	piapi_init( &cntx, PIAPI_MODE_AGENT, 0x0, argc, argv ); 

	while( 1 );

	piapi_destroy( &cntx );

	return 0;
}

