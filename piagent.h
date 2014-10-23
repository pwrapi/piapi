/* 
 * Copyright 2014 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work 
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 */

#ifndef PIAGENT_H
#define PIAGENT_H

/*! \fn int piapi_agent_init( void *cntx )
 *  \brief Initialize PIAPI agent
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *
 *  Initialization of the PIAPI agent accomplishes the following:
 *  - start listener on local well-known port (20201)
 *  - register local agent callback for results
 *  - initialize the local native interface to the device
 */
int piapi_agent_init( void *cntx );

/*! \fn int piapi_agent_destroy( void *cntx )
 *  \brief Destroy PIAPI agent
 *  \param cntx handle to context state
 *  \return 0 on success, negative on failure
 *
 *  Destruction of the PIAPI agent accomplishes the following:
 *  - shutdown the worker thread
 *  - close listener on local well-known port (20201)
 *  - destroy the local native interface to the device
 */
int piapi_agent_destroy( void *cntx );

#endif
