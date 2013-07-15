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
int piapi_native_destroy( void **cntx );

/*! \fn int piapi_native_collect( void *cntx )
 *  \brief Collect samples on a local native port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Collect samples on a local native sensor port at a given rate, reporting samples over the callback function
 */
int piapi_native_collect( void *cntx );

/*! \fn int piapi_native_counter( void *cntx )
 *  \brief Query counter on a local native port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Query counter on a local native sensor port, reporting sample over the callback function and passed parameter
 */
int piapi_native_counter( void *cntx, piapi_port_t port, piapi_sample_t *sample );

/*! \fn int piapi_native_clear( void *cntx, piapi_port_t port )
 *  \brief Clear counter on a local native port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Clear counter on a local native sensor port reseting all values to zero
 */
int piapi_native_clear( void *cntx );

#endif
