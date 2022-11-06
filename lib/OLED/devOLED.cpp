#include <Arduino.h>
#include "common.h"
#include "device.h"

#ifdef OLED
#include <U8g2lib.h>

U8G2_SH1107_128X80_F_HW_I2C u8g2(U8G2_R1, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/21);
void ClearBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h)
{
    uint8_t color = u8g2.getDrawColor();
    u8g2.setDrawColor(0);
    u8g2.drawBox(x, y, w, h);
    u8g2.setDrawColor(color);
}

static uint8_t gMaxPage = 3;
static uint8_t gCurPage = 0;

void OledPageDown(uint8_t num)
{
    gCurPage = (gCurPage + num)%gMaxPage;
}
#endif

static void initialize()
{
#ifdef OLED
    u8g2.begin();
    u8g2.enableUTF8Print(); // enable UTF8 support for the Arduino print() function

    u8g2.setFont(u8g2_font_unifont_t_chinese2); // use chinese2 for all the glyphs of "你好世界"
    u8g2.setFontDirection(0);
    u8g2.clearBuffer();

    u8g2.setCursor(0, 30);
    u8g2.print("init...");

    u8g2.setContrast(255);
#endif
}

static int start()
{
    return 200;
}

// static int event()
// {
//     return 0;
// }
#ifdef RELAY
  #include "telemetry.h"
  #include "crossfire.h"
  extern crsf_telemtry_data_s crsf_tlm_data;
#endif

static int timeout()
{
#ifdef OLED
    ClearBox(0, 0, 120, 80);
    bool result  = crsf_tlm_data.makeScreen(0);

    if(result) {
        for (size_t i = 0; i < 6; i++)
        {
            u8g2.setCursor(0, 12 * (i + 1));
            u8g2.print(crsf_tlm_data.oled_screen[i]);
        }
        
        // u8g2.print(pScreen);
    }
    // u8g2.print("9999");
    // u8g2.setCursor(0, 24);
    // u8g2.print("Cnt1:");
    // u8g2.print("9999");
    // u8g2.setCursor(0, 36);
    // u8g2.print("TlmCnt2:");
    // u8g2.print("9999");
    // u8g2.setCursor(0, 48);
    // u8g2.print("TlmCnt3:");
    // u8g2.print("9999");
    // u8g2.setCursor(0, 60);
    // u8g2.print("TlmCnt4:");
    // u8g2.print("9999");
    // u8g2.setCursor(0, 72);
    // u8g2.print("TlmCnt5:");
    // u8g2.print("9999");
    // u8g2.setCursor(80, 72);
    // u8g2.print("TTTT:");
    // u8g2.print("99");

    u8g2.sendBuffer();
#endif
    return 200;
}

device_t OLED_device = {
    .initialize = initialize,
    .start = start,
    // .event = event,
    .event = NULL,
    .timeout = timeout
};
