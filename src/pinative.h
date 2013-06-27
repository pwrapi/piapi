#ifndef PINATIVE_H
#define PINATIVE_H

#include "picommon.h"

/*! \fn int piapi_native_init( void *cntx )
 *  \brief Initialize native PIAPI
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *
 *  Initialization of the native PIAPI accomplishes the following:
 *  - start worker thread for counters 
 */
int piapi_native_init( void *cntx );

/*! \fn int piapi_native_destroy( void *cntx )
 *  \brief Destroy native PIAPI
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *
 *  Destruction of the native PIAPI accomplishes the following:
 *  - shutdown the worker thread
 *  - close the hardware devices
 */
int piapi_native_destroy( void *cntx );


int piapi_native_collect( void *cntx );
int piapi_native_counter( void *cntx, piapi_port_t port, piapi_sample_t *sample );
int piapi_native_clear( void *cntx, piapi_port_t port );

#endif
