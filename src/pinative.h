/* 
 * Copyright 2013-2015 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work 
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 */

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

/*! \fn int piapi_native_collect( void *cntx )
 *  \brief Collect samples on a local native port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Collect samples on a local native sensor port at a given rate, reporting samples over the callback function
 */
int piapi_native_collect( void *cntx );

/*! \fn int piapi_native_halt( void *cntx )
 *  \brief Halt collect samples
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Halt collect samples, stopping samples over the callback function
 */
int piapi_native_halt( void *cntx );

/*! \fn int piapi_native_counter( void *cntx )
 *  \brief Query counter on a local native port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Query counter on a local native sensor port, reporting sample over the callback function and passed parameter
 */
int piapi_native_counter( void *cntx );

/*! \fn int piapi_native_clear( void *cntx, piapi_port_t port )
 *  \brief Reset counter on a local native port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Reset counter on a local native sensor port reseting all values to zero
 */
int piapi_native_reset( void *cntx );

/*! \fn int piapi_native_log( void *cntx )
 *  \brief Control frequency of counter logging
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Control the frequency that the counter logs are written with zero indicating off
 */
int piapi_native_log( void *cntx );

/*! \fn int piapi_native_mark( void *cntx )
 *  \brief Insert a marker into counter log
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Insert a text marker to into counter sample log
 */
int piapi_native_mark( void *cntx );

/*! \fn int piapi_native_train( void *cntx )
 *  \brief Train prediction of a counter on a given port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Train the port based on the counter buffers
 */
int piapi_native_train( void *cntx );

/*! \fn int piapi_native_detect( void *cntx )
 *  \brief Detect average frequency and length of a counter on a given port
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Detect the period and duty cycle based on the counter buffers
 */
int piapi_native_detect( void *cntx );

/*! \fn int piapi_native_predict( void *cntx )
 *  \brief Predict the port with the greatest variation and length of phase
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *  
 *  Detect the period and duty cycle based on the counter buffers
 */
int piapi_native_predict( void *cntx );

#endif
