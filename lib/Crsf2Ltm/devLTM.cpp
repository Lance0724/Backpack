#include <Arduino.h>
#include "common.h"
#include "device.h"
#include "ltm.h"
#include "crossfire.h"
#include "devLED.h"
#if defined(PLATFORM_ESP8266)
  #include <espnow.h>
  #include <ESP8266WiFi.h>
#elif defined(PLATFORM_ESP32)
  #include <esp_now.h>
  #include <esp_wifi.h>
  #include <WiFi.h>
#endif

extern crsf_telemtry_data_s crsf_tlm_data;

#ifdef MY_UID
uint8_t LTMbroadcastAddress[6] = {MY_UID};
#else
uint8_t LTMbroadcastAddress[6] = {0, 0, 0, 0, 0, 0};
#endif


static void initialize()
{
    // crsf_tlm_data.init();
}

static int start()
{
    // 200ms begin to refresh
    return 200;
}

void sendLTMViaEspnow(uint8_t *data, size_t len)
{
  esp_now_send(LTMbroadcastAddress, data, len);

  blinkLED();
}

static int timeout()
{
    uint32_t now = millis();
    static uint8_t ltm_scheduler;

    if (500 < now - crsf_tlm_data.last_update) {
        return 100;
    }

    static uint8_t ltmFrame[LTM_MAX_MESSAGE_SIZE];
    int frameSize;

    if (ltm_scheduler & 1) {
        frameSize = getLtmFrame(ltmFrame, LTM_GFRAME);
        sendLTMViaEspnow(ltmFrame, frameSize);
    }
    frameSize = getLtmFrame(ltmFrame, LTM_AFRAME);
    sendLTMViaEspnow(ltmFrame, frameSize);

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
    .start = start,
    .event = NULL,
    .timeout = timeout
};
