#include <Arduino.h>
#include "common.h"
#include "device.h"

#if defined(PIN_BUTTON)
#include "logging.h"
#include "button.h"

static Button<PIN_BUTTON, false> button;

extern unsigned long rebootTime;
void RebootIntoWifi();

static void longPress()
{
    if (connectionState == wifiUpdate)
    {
        rebootTime = millis();
    }
    else
    {
        RebootIntoWifi();
    }
}

#ifdef OLED
extern void OledPageDown(uint8_t num);
#endif
static void shortPress()
{
#ifdef OLED
    OledPageDown(1);
#endif
}

static void initialize()
{
    button.OnLongPress = longPress;
    button.OnShortPress = shortPress;
}

static int start()
{
    return DURATION_IMMEDIATELY;
}

static int timeout()
{
    return button.update();
}

device_t Button_device = {
    .initialize = initialize,
    .start = start,
    .event = NULL,
    .timeout = timeout
};

#endif