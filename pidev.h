/* 
 * Copyright 2014 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this work 
 * by or on behalf of the U.S. Government. Export of this program may require
 * a license from the United States Government.
 */

#ifndef __PIDEV_H__
#define __PIDEV_H__

/* getRawPower9.2.c    (version 6.2)
 * For use with Carrier Board 10016423 Rev E8 (= Rev A)
 */

#include <stdint.h>

#define MAX_PORTNUM      15

typedef struct reading {
    uint16_t    Asamp;          // Raw sample
    uint16_t    Vsamp;          // Raw sample
    int32_t     miliamps;       // Calculated value
    int32_t     milivolts;      // Calculated value
    int32_t     miliwatts;      // Calculated value
} reading_t;

void pidev_read(int portNumber, reading_t *sample);
void pidev_close(void);

#endif
