#ifndef PICOMMON_H
#define PICOMMON_H

typedef enum {
        PIAPI_PORT_UNKNOWN = 0,
        PIAPI_PORT_CPU = 1,
        PIAPI_PORT_R1_12V = 1,
        PIAPI_PORT_12V = 2,
        PIAPI_PORT_R1_3_3V = 2,
        PIAPI_PORT_MEM = 3,
        PIAPI_PORT_R2_12V = 3,
        PIAPI_PORT_5V = 4,
        PIAPI_PORT_R2_3_3V = 4,
        PIAPI_PORT_3_3V = 5,
        PIAPI_PORT_ATX_12V = 5,
        PIAPI_PORT_HDD_12V = 6,
        PIAPI_PORT_R3_12V = 6,
        PIAPI_PORT_HDD_5V = 7,
        PIAPI_PORT_R3_3_3V = 7,
	PIAPI_PORT_R4_12V = 8,
	PIAPI_PORT_R4_3_3V = 9,
	PIAPI_PORT_C1_CPU_12V = 10,
	PIAPI_PORT_C1_MEM_12V = 11,
	PIAPI_PORT_C2_CPU_12V = 12,
	PIAPI_PORT_C2_MEM_12V = 13,
	PIAPI_PORT_ATX_3_3V = 14,
	PIAPI_PORT_OPT_12V = 15,
	PIAPI_PORT_MIN = PIAPI_PORT_R1_12V,
	PIAPI_PORT_MAX = PIAPI_PORT_OPT_12V,
	PIAPI_PORT_ALL = 16
} piapi_port_t;

typedef enum {
	PIAPI_MODE_UNKNOWN = 0,
	PIAPI_MODE_NATIVE,
	PIAPI_MODE_PROXY,
	PIAPI_MODE_AGENT
} piapi_mode_t;

typedef struct piapi_reading {
	float volts;
	float amps;
	float watts;
} piapi_reading_t;

typedef struct piapi_sample {
	void *cntx;
        unsigned int number;
        unsigned int total;
        unsigned long time_sec;
        unsigned long time_usec;
        unsigned int port;
	piapi_reading_t raw;
        piapi_reading_t min;
        piapi_reading_t max;
        piapi_reading_t avg;
	float time_total;
        float energy;
} piapi_sample_t;

typedef void (*piapi_callback_t)( piapi_sample_t *);

#endif
