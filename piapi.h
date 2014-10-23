/* 
 * Copyright 2014 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work 
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 */

#ifndef PIAPI_H
#define PIAPI_H

#include "picommon.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \fn int piapi_init( void *cntx, piapi_mode_t mode, piapi_callback_t callback,
 *   unsigned int saddr, unsigned short sport, unsigned int counterfreq );
 *  \brief Initialize PIAPI
 *  \param cntx handle to context state
 *  \param mode configuration type (native, proxy, agent)
 *  \param callback results callback function
 *  \param saddr socket ip address
 *  \param sport socket port
 *  \param frequency internal counter frequency
 *  \return 0 on success, negative on failure
 *
 *  Initialization of the PIAPI accomplishes the following:
 *  - sets mode and callback for the context
 *  - calls the appropriate initializer for the configuration
 */
int piapi_init( void **cntx, piapi_mode_t mode, piapi_callback_t callback,
    unsigned int saddr, unsigned short sport, unsigned int counterfreq );

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

/*! \fn int piapi_halt( void *cntx, piapi_port_t port )
 *  \brief Halting collection on a port
 *  \param cntx handle to context state
 *  \param port sensor port to halt collection on
 *  \return 0 on success, negative on failure
 *  
 *  Halt collection on a sensor port, stopping reporting sample over the callback function
 */
int piapi_halt( void *cntx, piapi_port_t port );

/*! \fn int piapi_counter( void *cntx, piapi_port_t port, piapi_sample_t *sample )
 *  \brief Query counter on a port
 *  \param cntx handle to context state
 *  \param port sensor port to query counter on
 *  \return 0 on success, negative on failure
 *  
 *  Query counter on a sensor port, reporting sample over the callback function
 */
int piapi_counter( void *cntx, piapi_port_t port );

/*! \fn int piapi_reset( void *cntx, piapi_port_t port )
 *  \brief Reset counter on a port
 *  \param cntx handle to context state
 *  \param port sensor port to query counter on
 *  \return 0 on success, negative on failure
 *  
 *  Reset counter on a sensor port reseting all values to zero
 */
int piapi_reset( void *cntx, piapi_port_t port );

/*! \fn int piapi_log( void *cntx, piapi_port_t port, unsigned int frequency )
 *  \brief Control frequency of counter logging
 *  \param cntx handle to context state
 *  \param port sensor port to control counter log on
 *  \param frequency number of samples per second
 *  \return 0 on success, negative on failure
 *  
 *  Control the frequency that the remote agent counter logs are written with zero indicating off
 */
int piapi_log( void *cntx, piapi_port_t port, unsigned int frequency );

/*! \fn int piapi_mark( void *cntx, char *marker )
 *  \brief Insert a marker into counter log
 *  \param cntx handle to context state
 *  \param marker string to insert into log
 *  \return 0 on success, negative on failure
 *  
 *  Insert a text marker to into the remote agent counter sample log
 */
int piapi_mark( void *cntx, char *marker );

/*! \fn int piapi_info( piapi_version_t *version )
 *  \brief Retrieve version information
 *  \param cntx handle to context state
 *  \param version information regarding version
 *  \return 0 on success, negative on failure
 *  
 *  Retrieve version information on piapi API
 */
int piapi_info( piapi_version_t *version );

#ifdef __cplusplus
}
#endif

#endif
