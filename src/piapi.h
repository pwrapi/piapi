#ifndef PIAPI_H
#define PIAPI_H

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

int piapi_collect( piapi_port_t port, piapi_reading_t *reading );
int piapi_close( void );

#endif
