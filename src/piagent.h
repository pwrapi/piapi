#ifndef PIAGENT_H
#define PIAGENT_H

#include "picommon.h"

int piapi_agent_init( void *cntx );
int piapi_agent_destroy( void *cntx );

int piapi_agent_collect( void *cntx );

#endif
