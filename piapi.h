#ifndef PIAPI_H
#define PIAPI_H

#include "picommon.h"

/*! \fn int piapi_init( void *cntx, piapi_mode_t mode, piapi_callback_t callback )
 *  \brief Initialize PIAPI
 *  \param cntx handle to context state
 *  \param mode configuration type (native, proxy, agent)
 *  \param callback results callback function
 *  \param saddr socket ip address
 *  \param sport socket port
 *  \return 0 on success, negative on failure
 *
 *  Initialization of the PIAPI accomplishes the following:
 *  - sets mode and callback for the context
 *  - calls the appropriate initializer for the configuration
 */
int piapi_init( void **cntx, piapi_mode_t mode, piapi_callback_t callback, unsigned int saddr, unsigned short sport );

/*! \fn int piapi_destroy( void *cntx )
 *  \brief Destroy PIAPI
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *
 *  Destruction of the PIAPI accomplishes the following:
 *  - calls the appropriate destructor for the configuration
 */
int piapi_destroy( void **cntx );

/*! \fn int piapi_collect( void *cntx, piapi_port_t port, unsigned int samples, unsigned int frequency )
 *  \brief Collect samples on a port
 *  \param cntx handle to context state
 *  \param port sensor port to collect on
 *  \param samples number of samples to take
 *  \param frequency number of samples per second
 *  \return 0 on success, negative on failure
 *  
 *  Collect samples on a sensor port at a given rate, reporting samples over the callback function
 */
int piapi_collect( void *cntx, piapi_port_t port, unsigned int samples, unsigned int frequency );

/*! \fn int piapi_counter( void *cntx, piapi_port_t port, piapi_sample_t *sample )
 *  \brief Query counter on a port
 *  \param cntx handle to context state
 *  \param port sensor port to query counter on
 *  \return 0 on success, negative on failure
 *  
 *  Query counter on a sensor port, reporting sample over the callback function
 */
int piapi_counter( void *cntx, piapi_port_t port );

/*! \fn int piapi_clear( void *cntx, piapi_port_t port )
 *  \brief Reset counter on a port
 *  \param cntx handle to context state
 *  \param port sensor port to query counter on
 *  \return 0 on success, negative on failure
 *  
 *  Reset counter on a sensor port reseting all values to zero
 */
int piapi_reset( void *cntx, piapi_port_t port );

#endif
