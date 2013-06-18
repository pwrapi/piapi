#ifndef PIAPI_H
#define PIAPI_H

// The well-known powerinsight agent saddr
#define PIAPI_AGNT_SADDR    0x0a361500

// The well-known powerinsight agent port
#define PIAPI_AGNT_PORT     20201

typedef enum {
        PIAPI_UNKNOWN = 0,
        PIAPI_CPU = 1,
        PIAPI_12V = 2,
        PIAPI_MEM = 3,
        PIAPI_5V = 4,
        PIAPI_3_3V = 5,
        PIAPI_HDD_12V = 6,
        PIAPI_HDD_5V = 7,
        PIAPI_ALL = 8
} piapi_port_t;

typedef struct piapi_reading {
    float volts;
    float amps;
    float watts;
} piapi_reading_t;

typedef struct piapi_sample {
        unsigned int number;
        unsigned int total;
        unsigned long time_sec;
        unsigned long time_usec;
        float power;
        float energy;
} piapi_sample_t;

typedef void (*piapi_callback_t)( piapi_sample_t *);

/* Proxy calls */
int
picomm_proxy_init( void **cntx, piapi_callback_t callback );
int
piapi_proxy_collect( void *cntx, piapi_port_t port, unsigned int samples, unsigned int frequency );
int
piapi_proxy_destroy( void *cntx );

/* Agent calls */
int
picomm_agent_init( void **cntx );
int
picomm_agent_destroy( void **cntx );

/* Device calls */
int
piapi_dev_collect( piapi_port_t port, piapi_reading_t *reading );
int
piapi_dev_close( void );

#endif
