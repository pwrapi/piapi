#ifndef PIPROXY_H
#define PIPROXY_H

void piapi_proxy_thread( void *cntx );
int piapi_proxy_connect( void *cntx );
int piapi_proxy_collect( void *cntx );

int piapi_proxy_counter( void *cntx );
int piapi_proxy_clear( void *cntx );

#endif
