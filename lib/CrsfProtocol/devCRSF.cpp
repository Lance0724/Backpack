#include <Arduino.h>
#include "common.h"
#include "device.h"
#include "crossfire.h"

extern crsf_telemtry_data_s crsf_tlm_data;

static void initialize()
{
    crsf_tlm_data.init();
}

static int start()
{
    // 200ms begin to refresh
    return 200;
}

static int timeout()
{
    crsf_tlm_data.makeScreen(0);
    return 200;
}

// static int event()
// {
//     return DURATION_NEVER;
// }

device_t CRSF_device = {
    .initialize = initialize,
    // .start = event,
    .start = NULL,
    .event = start,
    .timeout = timeout
};
