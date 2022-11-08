#include <Arduino.h>
#include "common.h"
#include "device.h"
#include "ltm.h"

static void initialize()
{
    // crsf_tlm_data.init();
}

static int start()
{
    // 200ms begin to refresh
    return 100;
}

static int timeout()
{
    static uint8_t ltm_scheduler;

    // bool result  = crsf_tlm_data.makeScreen(0);
    static uint8_t ltmFrame[LTM_MAX_MESSAGE_SIZE];
    int frameSize;
    if (ltm_scheduler & 1) {
        frameSize = getLtmFrame(ltmFrame, LTM_GFRAME);
        // send
    }
    frameSize = getLtmFrame(ltmFrame, LTM_AFRAME);
    // send

    ltm_scheduler++;
    ltm_scheduler %= 10;

    return 100;
}

// static int event()
// {
//     return DURATION_NEVER;
// }

device_t LTM_device = {
    .initialize = initialize,
    // .start = event,
    .start = NULL,
    .event = start,
    .timeout = timeout
};
