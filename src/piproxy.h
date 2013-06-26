#ifndef PIPROXY_H
#define PIPROXY_H

int piapi_proxy_init( void *cntx );
int piapi_proxy_destroy( void *cntx );

int piapi_proxy_collect( void *cntx );
int piapi_proxy_counter( void *cntx );
int piapi_proxy_clear( void *cntx );

#endif
