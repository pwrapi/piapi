/* 
 * Copyright 2014 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work 
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 */

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

/*! \fn int piapi_proxy_collect( void *cntx, piapi_port_t port, unsigned int samples, unsigned int frequency )
 *  \brief Collect samples on a remote agent port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Collect samples on a remote agent sensor port at a given rate, reporting samples over the callback function
 */
int piapi_proxy_collect( void *cntx );

/*! \fn int piapi_proxy_halt( void *cntx )
 *  \brief Halt collect samples on remote agent port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Halt collect samples on remote agent sensor port,, stopping samples over the callback function
 */
int piapi_proxy_halt( void *cntx );

/*! \fn int piapi_proxy_counter( void *cntx )
 *  \brief Query counter on a remote agent port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Query counter on a remote agent sensor port, reporting sample over the callback function
 */
int piapi_proxy_counter( void *cntx );

/*! \fn int piapi_proxy_reset( void *cntx )
 *  \brief reset counter on a remote agent port
 *  \param cntx handle to context state
 *  \param port sensor port to query counter on
 *  \return 0 on success, negative on failure
 *  
 *  Reset counter on a remote agent sensor port reseting all values to zero
 */
int piapi_proxy_reset( void *cntx );

/*! \fn int piapi_proxy_log( void *cntx )
 *  \brief Control frequency of counter logging
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Control the frequency that the remote agent counter logs are written with zero indicating off
 */
int piapi_proxy_log( void *cntx );

/*! \fn int piapi_proxy_mark( void *cntx )
 *  \brief Insert a marker into counter log
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Insert a text marker to into the remote agent counter sample log
 */
int piapi_proxy_mark( void *cntx );

/*! \fn int piapi_proxy_train( void *cntx )
 *  \brief Train prediction of a counter on a given port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Train the port based on the remote agent counter buffers
 */
int piapi_proxy_train( void *cntx );

/*! \fn int piapi_proxy_detect( void *cntx )
 *  \brief Detect average frequency and length of a counter on a given port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Detect the period and duty cycle based on the remote agent counter buffers
 */
int piapi_proxy_detect( void *cntx );

/*! \fn int piapi_proxy_predict( void *cntx )
 *  \brief Predict the port with the greatest variation and length of phase
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Predict the period and duty cycle based on the remote agent counter buffers
 */
int piapi_proxy_predict( void *cntx );

#endif
