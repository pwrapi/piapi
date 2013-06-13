#ifndef __PIAPI_H__
#define __PIAPI_H__

/* getRawPower9.2.c    (version 6.2)
 * For use with Carrier Board 10016423 Rev E8 (= Rev A)
 */

#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PORTNUM      15

typedef struct reading {
    uint16_t    Asamp;          // Raw sample
    uint16_t    Vsamp;          // Raw sample
    int32_t     miliamps;       // Calculated value
    int32_t     milivolts;      // Calculated value
    int32_t     miliwatts;      // Calculated value
} reading_t;

void getReadings(int portNumber, reading_t *sample);
void calcValues(int portNumber, reading_t *sample);

void closePorts(void);

#endif
