#include "crossfire.h"

extern GENERIC_CRC8 crsf_crc;

crsf_telemetry_data_s crsf_tlm_data;

#define TELEMETRY_RX_PACKET_SIZE       128
// static bool _lenIsSane(uint8_t len)
// {
//   // packet len must be at least 3 bytes (type+payload+crc) and 2 bytes < MAX (hdr+len)
//   return (len > 2 && len < TELEMETRY_RX_PACKET_SIZE-1);
// }

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

void processCrossfireTelemetryFrame(uint8_t nextPayloadSize, uint8_t *payloadData, uint32_t now)
{
  bool bTelemData = true;
  uint8_t posCount = 0;
  uint8_t *rxBuffer = payloadData;

  uint8_t crsfPayloadLen = rxBuffer[1];
  uint8_t id = rxBuffer[2];
  int32_t value;
  switch(id) {
    case CRSF_FRAMETYPE_VARIO:
      // if (getCrossfireTelemetryValue<2>(3, value, payloadData))
      //   processCrossfireTelemetryValue(VERTICAL_SPEED_INDEX, value);
      break;

    case CRSF_FRAMETYPE_GPS:
      if (getCrossfireTelemetryValue<4>(3, value, payloadData))
      {
        crsf_tlm_data.telemetry_lat = value / 10;
        if (posCount == 0) {
          posCount++;
        }
      }
      if (getCrossfireTelemetryValue<4>(7, value, payloadData))
      {
        crsf_tlm_data.telemetry_lon = value / 10;
        if (posCount == 0) {
          posCount++;
        }
      }
      if (getCrossfireTelemetryValue<2>(11, value, payloadData))
      {
        // (m/h +50m/h)/100
        crsf_tlm_data.telemetry_speed = (float)value;
      }
      if (getCrossfireTelemetryValue<2>(13, value, payloadData))
      {
        crsf_tlm_data.telemetry_course = (float)value;
      }
      if (getCrossfireTelemetryValue<2>(15, value, payloadData))
      {
        crsf_tlm_data.telemetry_alt = (uint16_t) (value - 1000);
        crsf_tlm_data.telemetry_gotAlt = true;
      }
      if (getCrossfireTelemetryValue<1>(17, value, payloadData))
      {
        crsf_tlm_data.telemetry_sats = (uint16_t)value;
      }
      break;
    case CRSF_FRAMETYPE_BATTERY_SENSOR:
      if (getCrossfireTelemetryValue<2>(3, value, payloadData)){
        crsf_tlm_data.telemetry_voltage = (uint16_t)value;
      }
      if (getCrossfireTelemetryValue<2>(5, value, payloadData)){
        crsf_tlm_data.telemetry_current = (float)value;

      }
      if (getCrossfireTelemetryValue<3>(7, value, payloadData)){
        // processCrossfireTelemetryValue(BATT_CAPACITY_INDEX, value);
      }
        
      if (getCrossfireTelemetryValue<1>(10, value, payloadData)){
        // processCrossfireTelemetryValue(BATT_REMAINING_INDEX, value);
      }
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

    case CRSF_FRAMETYPE_ATTITUDE:
      if (getCrossfireTelemetryValue<2>(3, value, payloadData))
      {
        // Euler angles = telemetry_pitch * 180.0f / (1000 * 3.1415...)
        crsf_tlm_data.telemetry_pitch = (int16_t)value;
      }
      //   processCrossfireTelemetryValue(ATTITUDE_PITCH_INDEX, value/10);
      if (getCrossfireTelemetryValue<2>(5, value, payloadData))
      {
        crsf_tlm_data.telemetry_roll = (int16_t)value;
      }
      //   processCrossfireTelemetryValue(ATTITUDE_ROLL_INDEX, value/10);
      if (getCrossfireTelemetryValue<2>(7, value, payloadData))
      {
        crsf_tlm_data.telemetry_yaw = (int16_t)value;
      }
      //   processCrossfireTelemetryValue(ATTITUDE_YAW_INDEX, value/10);
      break;

    case CRSF_FRAMETYPE_FLIGHT_MODE:
    {
      auto textLength = min<int>(16, crsfPayloadLen);
      memset(crsf_tlm_data.telemtry_flightMode, 0, sizeof(crsf_tlm_data.telemtry_flightMode));
      strncpy(crsf_tlm_data.telemtry_flightMode, (const char *)rxBuffer + 3, textLength);
    }
      break;

    default:
      bTelemData = false;
    break;
  }

  if (posCount == 2)
  {
    posCount = 0;
    crsf_tlm_data.telemetry_gotFix = true;
    // crsf_tlm_data.last_update = now;
    // printf("Sats/Lat/Lon/Alt: %d %d %d %d\n", telemetry_sats, telemetry_lat, telemetry_lon, telemetry_alt);
  }
  
  if (bTelemData == true) {
    crsf_tlm_data.last_update = now;
  }
}


static bool _checkFrameCRC(uint8_t* rxBuffer)
{
  uint8_t crc = crsf_crc.calc(rxBuffer + CRSF_FRAME_NOT_COUNTED_BYTES, rxBuffer[CRSF_TELEMETRY_LENGTH_INDEX] - CRSF_TELEMETRY_CRC_LENGTH);
  return (crc == rxBuffer[rxBuffer[CRSF_TELEMETRY_LENGTH_INDEX]+1]);
}

void crossfireProcessData(uint8_t nextPayloadSize, uint8_t *payloadData, uint32_t now)
{
    if (_checkFrameCRC(payloadData)) {
      processCrossfireTelemetryFrame(nextPayloadSize, payloadData, now);
    }
    else {
      // TRACE("[XF] CRC error ");
      // _seekStart(buffer, len); // adjusts len
    }
}
