#include "piapi.h"
#include "pidev.h"

int piapi_collect( piapi_port_t port, piapi_reading_t *reading )
{
    reading_t raw;
    pidev_read(port, &raw);

    reading->volts = raw.milivolts/1000.0;
    reading->amps = raw.miliamps/1000.0;
    reading->watts = raw.miliwatts/1000.0;

    return 0;
}

int piapi_close( void )
{
    pidev_close();

    return 0;
}

