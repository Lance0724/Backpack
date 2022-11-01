#include "crossfire.h"


extern GENERIC_CRC8 crsf_crc(CRSF_CRC_POLY);

const CrossfireSensor crossfireSensors[] = {
    {CRSF_FRAMETYPE_LINK_STATISTICS, 0, "1RSS", UNIT_DB, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS, 1, "2RSS", UNIT_DB, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS, 2, "RQly", UNIT_PERCENT, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS, 3, "RSNR", UNIT_DB, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS, 4, "ANT", UNIT_RAW, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS, 5, "RFMD", UNIT_RAW, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS, 6, "TPWR", UNIT_MILLIWATTS, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS, 7, "TRSS", UNIT_DB, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS, 8, "TQly", UNIT_PERCENT, 0},
    {CRSF_FRAMETYPE_LINK_STATISTICS, 9, "TSNR", UNIT_DB, 0},
    {CRSF_FRAMETYPE_BATTERY_SENSOR, 0, "RxBt", UNIT_VOLTS, 1},
    {CRSF_FRAMETYPE_BATTERY_SENSOR, 1, "Curr", UNIT_AMPS, 1},
    {CRSF_FRAMETYPE_BATTERY_SENSOR, 2, "Capa", UNIT_MAH, 0},
    {CRSF_FRAMETYPE_BATTERY_SENSOR, 3, "Bat%", UNIT_PERCENT, 0},
    {CRSF_FRAMETYPE_GPS, 0, "GPS", UNIT_GPS_LATITUDE, 0},
    {CRSF_FRAMETYPE_GPS, 0, "GPS", UNIT_GPS_LONGITUDE, 0},
    {CRSF_FRAMETYPE_GPS, 2, "GSpd", UNIT_KMH, 1},
    {CRSF_FRAMETYPE_GPS, 3, "Hdg", UNIT_DEGREE, 3},
    {CRSF_FRAMETYPE_GPS, 4, "Alt", UNIT_METERS, 0},
    {CRSF_FRAMETYPE_GPS, 5, "Sats", UNIT_RAW, 0},
    {CRSF_FRAMETYPE_ATTITUDE, 0, "Ptch", UNIT_RADIANS, 3},
    {CRSF_FRAMETYPE_ATTITUDE, 1, "Roll", UNIT_RADIANS, 3},
    {CRSF_FRAMETYPE_ATTITUDE, 2, "Yaw", UNIT_RADIANS, 3},
    {CRSF_FRAMETYPE_FLIGHT_MODE, 0, "FM", UNIT_TEXT, 0},
    {CRSF_FRAMETYPE_VARIO, 0, "VSpd", UNIT_METERS_PER_SECOND, 2},
    {CRSF_FRAMETYPE_BARO_ALTITUDE, 0, "Alt", UNIT_METERS, 2},
    {0, 0, "UNKNOWN", UNIT_RAW, 0},
};

// uint8_t crc8(const uint8_t * ptr, uint32_t len)
// {
//   uint8_t crc = 0;
//   for (uint32_t i=0; i<len; i++) {
//     crc = crc8tab[crc ^ *ptr++];
//   }
//   return crc;
// }


template<int N>
bool getCrossfireTelemetryValue(uint8_t index, int32_t & value, uint8_t module)
{
  uint8_t * rxBuffer = getTelemetryRxBuffer(module);
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

void processCrossfireTelemetryFrame(uint8_t* nextPayloadSize, uint8_t *payloadData)
{
  uint8_t * rxBuffer = payloadData;
  uint8_t &rxBufferCount = *nextPayloadSize;

  uint8_t crsfPayloadLen = rxBuffer[1];
  uint8_t id = rxBuffer[2];
  int32_t value;
  switch(id) {
    case CRSF_FRAMETYPE_VARIO:
      if (getCrossfireTelemetryValue<2>(3, value, module))
        processCrossfireTelemetryValue(VERTICAL_SPEED_INDEX, value);
      break;

    case CRSF_FRAMETYPE_GPS:
      if (getCrossfireTelemetryValue<4>(3, value, module))
        processCrossfireTelemetryValue(GPS_LATITUDE_INDEX, value/10);
      if (getCrossfireTelemetryValue<4>(7, value, module))
        processCrossfireTelemetryValue(GPS_LONGITUDE_INDEX, value/10);
      if (getCrossfireTelemetryValue<2>(11, value, module))
        processCrossfireTelemetryValue(GPS_GROUND_SPEED_INDEX, value);
      if (getCrossfireTelemetryValue<2>(13, value, module))
        processCrossfireTelemetryValue(GPS_HEADING_INDEX, value);
      if (getCrossfireTelemetryValue<2>(15, value, module))
        processCrossfireTelemetryValue(GPS_ALTITUDE_INDEX,  value - 1000);
      if (getCrossfireTelemetryValue<1>(17, value, module))
        processCrossfireTelemetryValue(GPS_SATELLITES_INDEX, value);
      break;

    case CRSF_FRAMETYPE_BARO_ALTITUDE:
      if (getCrossfireTelemetryValue<2>(3, value, module)) {
        if (value & 0x8000) {
          // Altitude in meters
          value &= ~(0x8000);
          value *= 100; // cm
        } else {
          // Altitude in decimeters + 10000dm
          value -= 10000;
          value *= 10;
        }
        processCrossfireTelemetryValue(BARO_ALTITUDE_INDEX, value);
      }
      // Length of TBS BARO_ALT has 4 payload bytes with just 2 bytes of altitude
      // but support including VARIO if the declared payload length is 6 bytes or more
      if (crsfPayloadLen > 5 && getCrossfireTelemetryValue<2>(5, value, module))
        processCrossfireTelemetryValue(VERTICAL_SPEED_INDEX, value);
      break;


    case CRSF_FRAMETYPE_BATTERY_SENSOR:
      if (getCrossfireTelemetryValue<2>(3, value, module))
        processCrossfireTelemetryValue(BATT_VOLTAGE_INDEX, value);
      if (getCrossfireTelemetryValue<2>(5, value, module))
        processCrossfireTelemetryValue(BATT_CURRENT_INDEX, value);
      if (getCrossfireTelemetryValue<3>(7, value, module))
        processCrossfireTelemetryValue(BATT_CAPACITY_INDEX, value);
      if (getCrossfireTelemetryValue<1>(10, value, module))
        processCrossfireTelemetryValue(BATT_REMAINING_INDEX, value);
      break;

    case CRSF_FRAMETYPE_ATTITUDE:
      if (getCrossfireTelemetryValue<2>(3, value, module))
        processCrossfireTelemetryValue(ATTITUDE_PITCH_INDEX, value/10);
      if (getCrossfireTelemetryValue<2>(5, value, module))
        processCrossfireTelemetryValue(ATTITUDE_ROLL_INDEX, value/10);
      if (getCrossfireTelemetryValue<2>(7, value, module))
        processCrossfireTelemetryValue(ATTITUDE_YAW_INDEX, value/10);
      break;

    case CRSF_FRAMETYPE_FLIGHT_MODE:
    {
      const CrossfireSensor & sensor = crossfireSensors[FLIGHT_MODE_INDEX];
      auto textLength = min<int>(16, rxBuffer[1]);
      rxBuffer[textLength] = '\0';
      setTelemetryText(PROTOCOL_TELEMETRY_CROSSFIRE, sensor.id, 0, sensor.subId,
                       (const char *)rxBuffer + 3);
      break;
    }

    default:
    break;
  }
}


static bool _checkFrameCRC(uint8_t* rxBuffer)
{
  uint8_t crc = crsf_crc.calc(rxBuffer + CRSF_FRAME_NOT_COUNTED_BYTES, rxBuffer[CRSF_TELEMETRY_LENGTH_INDEX] - CRSF_TELEMETRY_CRC_LENGTH);
  return (crc == rxBuffer[rxBuffer[CRSF_TELEMETRY_LENGTH_INDEX]+1]);
}

static void crossfireProcessData(void* context, uint8_t data, uint8_t* buffer, uint8_t* len)
{
  if (*len == 0 && data != CRSF_ADDRESS_RADIO_TRANSMITTER && data != CRSF_SYNC_BYTE) {
    // TRACE("[XF] address 0x%02X error", data);
    return;
  }

  if (*len == 1 && !_lenIsSane(data)) {
    // TRACE("[XF] length 0x%02X error", data);
    *len = 0;
    return;
  }

  if (*len < TELEMETRY_RX_PACKET_SIZE) {
    buffer[(*len)++] = data;
  }
  else {
    TRACE("[XF] array size %d error", *len);
    *len = 0;
  }

  // rxBuffer[1] holds the packet length-2, check if the whole packet was received
  while (*len > 4 && (buffer[1]+2) == *len) {
    // TODO: module in context?
    if (_checkFrameCRC(buffer)) {
#if defined(BLUETOOTH)
      if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY &&
          bluetooth.state == BLUETOOTH_STATE_CONNECTED) {
        bluetooth.write(buffer, *len);
      }
#endif
      auto state = (CrossfireState*)context;
      processCrossfireTelemetryFrame(state->module);
      *len = 0;
    }
    else {
      TRACE("[XF] CRC error ");
      _seekStart(buffer, len); // adjusts len
    }
  }
}
