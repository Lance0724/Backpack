#include "crossfire.h"
#ifdef OLED
  #include <U8g2lib.h>
#endif


extern GENERIC_CRC8 crsf_crc;

extern U8G2_SH1107_128X80_F_HW_I2C u8g2;

extern int32_t telemetry_lat;
extern int32_t telemetry_lon;
extern int16_t telemetry_alt;
extern int16_t telemetry_sats;
extern int32_t telemetry_time;
extern int32_t telemetry_date;
extern int16_t telemetry_age;


extern uint8_t telemetry_failed_cs;

extern float telemetry_course;
extern float telemetry_speed;
extern float telemetry_declinationf;
extern float telemetry_hdop;

extern float telemetry_pitch;
extern float telemetry_roll;
extern float telemetry_yaw;

#define TELEMETRY_RX_PACKET_SIZE       128
static bool _lenIsSane(uint8_t len)
{
  // packet len must be at least 3 bytes (type+payload+crc) and 2 bytes < MAX (hdr+len)
  return (len > 2 && len < TELEMETRY_RX_PACKET_SIZE-1);
}

const CrossfireSensor crossfireSensors[] = {
    {CRSF_FRAMETYPE_LINK_STATISTICS,  0, "1RSS", UNIT_DB, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS,  1, "2RSS", UNIT_DB, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS,  2, "RQly", UNIT_PERCENT, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS,  3, "RSNR", UNIT_DB, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS,  4, "ANT", UNIT_RAW, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS,  5, "RFMD", UNIT_RAW, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS,  6, "TPWR", UNIT_MILLIWATTS, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS,  7, "TRSS", UNIT_DB, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS,  8, "TQly", UNIT_PERCENT, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS,  9, "TSNR", UNIT_DB, 0},
    {CRSF_FRAMETYPE_BATTERY_SENSOR,   0, "RxBt", UNIT_VOLTS, 1},
    {CRSF_FRAMETYPE_BATTERY_SENSOR,   1, "Curr", UNIT_AMPS, 1},
    {CRSF_FRAMETYPE_BATTERY_SENSOR,   2, "Capa", UNIT_MAH, 0},
    {CRSF_FRAMETYPE_BATTERY_SENSOR,   3, "Bat%", UNIT_PERCENT, 0},
    {CRSF_FRAMETYPE_GPS,              0, "GPS", UNIT_GPS_LATITUDE, 0},
    {CRSF_FRAMETYPE_GPS,              0, "GPS", UNIT_GPS_LONGITUDE, 0},
    {CRSF_FRAMETYPE_GPS,              2, "GSpd", UNIT_KMH, 1},
    {CRSF_FRAMETYPE_GPS,              3, "Hdg", UNIT_DEGREE, 3},
    {CRSF_FRAMETYPE_GPS,              4, "Alt", UNIT_METERS, 0},
    {CRSF_FRAMETYPE_GPS,              5, "Sats", UNIT_RAW, 0},
    {CRSF_FRAMETYPE_ATTITUDE,         0, "Ptch", UNIT_RADIANS, 3},
    {CRSF_FRAMETYPE_ATTITUDE,         1, "Roll", UNIT_RADIANS, 3},
    {CRSF_FRAMETYPE_ATTITUDE,         2, "Yaw", UNIT_RADIANS, 3},
    {CRSF_FRAMETYPE_FLIGHT_MODE,      0, "FM", UNIT_TEXT, 0},
    {CRSF_FRAMETYPE_VARIO,            0, "VSpd", UNIT_METERS_PER_SECOND, 2},
    {CRSF_FRAMETYPE_BARO_ALTITUDE,    0, "Alt", UNIT_METERS, 2},
    {0, 0, "UNKNOWN", UNIT_RAW, 0},
};

template<int N>
bool getCrossfireTelemetryValue(uint8_t index, int32_t & value, uint8_t *payloadData)
{
  uint8_t * rxBuffer = payloadData;
  bool result = false;
  uint8_t * byte = &rxBuffer[index];
  value = (*byte & 0x80) ? -1 : 0;
  for (uint8_t i=0; i<N; i++) {
    value <<= 8;
    if (*byte != 0xff) {
      result = true;
    }
    value += *byte++;
  }
  return result;
}
#ifdef OLED
extern void ClearBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h);
#endif

void processCrossfireTelemetryFrame(uint8_t nextPayloadSize, uint8_t *payloadData)
{
  uint8_t * rxBuffer = payloadData;
  uint8_t &rxBufferCount = nextPayloadSize;

  uint8_t crsfPayloadLen = rxBuffer[1];
  uint8_t id = rxBuffer[2];
  int32_t value;
  switch(id) {
    case CRSF_FRAMETYPE_VARIO:
      // if (getCrossfireTelemetryValue<2>(3, value, payloadData))
      //   processCrossfireTelemetryValue(VERTICAL_SPEED_INDEX, value);
      break;

    case CRSF_FRAMETYPE_GPS:
      // if (getCrossfireTelemetryValue<4>(3, value, payloadData))
      //   processCrossfireTelemetryValue(GPS_LATITUDE_INDEX, value/10);
      // if (getCrossfireTelemetryValue<4>(7, value, payloadData))
      //   processCrossfireTelemetryValue(GPS_LONGITUDE_INDEX, value/10);
      // if (getCrossfireTelemetryValue<2>(11, value, payloadData))
      //   processCrossfireTelemetryValue(GPS_GROUND_SPEED_INDEX, value);
      // if (getCrossfireTelemetryValue<2>(13, value, payloadData))
      //   processCrossfireTelemetryValue(GPS_HEADING_INDEX, value);
      // if (getCrossfireTelemetryValue<2>(15, value, payloadData))
      //   processCrossfireTelemetryValue(GPS_ALTITUDE_INDEX,  value - 1000);
      // if (getCrossfireTelemetryValue<1>(17, value, payloadData))
      //   processCrossfireTelemetryValue(GPS_SATELLITES_INDEX, value);
      break;

    case CRSF_FRAMETYPE_BARO_ALTITUDE:
      // if (getCrossfireTelemetryValue<2>(3, value, payloadData)) {
      //   if (value & 0x8000) {
      //     // Altitude in meters
      //     value &= ~(0x8000);
      //     value *= 100; // cm
      //   } else {
      //     // Altitude in decimeters + 10000dm
      //     value -= 10000;
      //     value *= 10;
      //   }
      //   processCrossfireTelemetryValue(BARO_ALTITUDE_INDEX, value);
      // }
      // // Length of TBS BARO_ALT has 4 payload bytes with just 2 bytes of altitude
      // // but support including VARIO if the declared payload length is 6 bytes or more
      // if (crsfPayloadLen > 5 && getCrossfireTelemetryValue<2>(5, value, payloadData))
      // {
      //   processCrossfireTelemetryValue(VERTICAL_SPEED_INDEX, value);
      // }
      break;


    case CRSF_FRAMETYPE_BATTERY_SENSOR:
      // if (getCrossfireTelemetryValue<2>(3, value, payloadData))
      //   processCrossfireTelemetryValue(BATT_VOLTAGE_INDEX, value);
      // if (getCrossfireTelemetryValue<2>(5, value, payloadData))
      //   processCrossfireTelemetryValue(BATT_CURRENT_INDEX, value);
      // if (getCrossfireTelemetryValue<3>(7, value, payloadData))
      //   processCrossfireTelemetryValue(BATT_CAPACITY_INDEX, value);
      // if (getCrossfireTelemetryValue<1>(10, value, payloadData))
      //   processCrossfireTelemetryValue(BATT_REMAINING_INDEX, value);
      break;

    case CRSF_FRAMETYPE_ATTITUDE:
      // if (getCrossfireTelemetryValue<2>(3, value, payloadData))
      //   processCrossfireTelemetryValue(ATTITUDE_PITCH_INDEX, value/10);
      // if (getCrossfireTelemetryValue<2>(5, value, payloadData))
      //   processCrossfireTelemetryValue(ATTITUDE_ROLL_INDEX, value/10);
      // if (getCrossfireTelemetryValue<2>(7, value, payloadData))
      //   processCrossfireTelemetryValue(ATTITUDE_YAW_INDEX, value/10);
      break;

    case CRSF_FRAMETYPE_FLIGHT_MODE:
    {
      // const CrossfireSensor & sensor = crossfireSensors[FLIGHT_MODE_INDEX];
      // auto textLength = min<int>(16, rxBuffer[1]);
      // rxBuffer[textLength] = '\0';
      // setTelemetryText(PROTOCOL_TELEMETRY_CROSSFIRE, sensor.id, 0, sensor.subId,
      //                  (const char *)rxBuffer + 3);
      
    }
      break;

    default:
    break;
  }
#ifdef OLED
  // ClearBox(0, 25, 90, 10);
  u8g2.setCursor(0, 35);
  u8g2.print("ID:");
  u8g2.printf("0x%x", id);
#endif
}


static bool _checkFrameCRC(uint8_t* rxBuffer)
{
  uint8_t crc = crsf_crc.calc(rxBuffer + CRSF_FRAME_NOT_COUNTED_BYTES, rxBuffer[CRSF_TELEMETRY_LENGTH_INDEX] - CRSF_TELEMETRY_CRC_LENGTH);
  return (crc == rxBuffer[rxBuffer[CRSF_TELEMETRY_LENGTH_INDEX]+1]);
}

void crossfireProcessData(uint8_t nextPayloadSize, uint8_t *payloadData)
{
    if (_checkFrameCRC(payloadData)) {
      #ifdef OLED
        u8g2.setCursor(0, 55);
        u8g2.print("Ok");
      #endif
      processCrossfireTelemetryFrame(nextPayloadSize, payloadData);
    }
    else {
      // TRACE("[XF] CRC error ");
      // _seekStart(buffer, len); // adjusts len
      #ifdef OLED
        u8g2.setCursor(0, 55);
        u8g2.print("Bad");
      #endif
    }
}
