#ifndef PIPROXY_H
#define PIPROXY_H

/*! \fn int piapi_proxy_init( void *cntx )
 *  \brief Initialize PIAPI proxy
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *
 *  Initialization of the PIAPI agent accomplishes the following:
 *  - start connection to agent at well-known port (20201)
 *  - start worker thread to service reads from agent
 */
int piapi_proxy_init( void *cntx );

/*! \fn int piapi_proxy_destroy( void *cntx )
 *  \brief Destroy PIAPI proxy
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *
 *  Destruction of the native PIAPI accomplishes the following:
 *  - shutdown the worker thread
 *  - close the connection to agent
 */
int piapi_proxy_destroy( void *cntx );

int piapi_proxy_collect( void *cntx );
int piapi_proxy_counter( void *cntx );
int piapi_proxy_clear( void *cntx );

#endif
