#ifndef PIAPI_H
#define PIAPI_H

#include "picommon.h"

/*! \fn int piapi_init( void *cntx, piapi_mode_t mode, piapi_callback_t callback )
 *  \brief Initialize PIAPI
 *  \param cntx handle to context state
 *  \param mode configuration type (native, proxy, agent)
 *  \param callback results callback function
 *  \return 0 on success, negative on failure
 *
 *  Initialization of the PIAPI accomplishes the following:
 *  - 
 *  - 
 *  - 
 */
int piapi_init( void **cntx, piapi_mode_t mode, piapi_callback_t callback );

/*! \fn int piapi_destroy( void *cntx )
 *  \brief Destroy PIAPI agent
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *
 *  Destruction of the PIAPI accomplishes the following:
 *  - 
 *  - 
 *  - 
 */
int piapi_destroy( void *cntx );


int piapi_collect( void *cntx, piapi_port_t port, unsigned int samples, unsigned int frequency );


int piapi_counter( void *cntx, piapi_port_t port, piapi_sample_t *sample );
int piapi_clear( void *cntx, piapi_port_t port );

#endif
