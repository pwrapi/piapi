#ifndef __PIDEV_H__
#define __PIDEV_H__

/* getRawPower9.2.c    (version 6.2)
 * For use with Carrier Board 10016423 Rev E8 (= Rev A)
 */

#include <stdint.h>

#define MAX_PORTNUM      15

typedef struct {
    union { double  reading, watt, temp ; };
    double  volt ;  /* Voltage component of Watt measurement */
    double  amp ;  /* Amperage component */
} reading_t ;

void pidev_read(int portNumber, reading_t *sample);
void pidev_open(void);
void pidev_close(void);

#endif
