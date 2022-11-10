#pragma once
#include "crsf_protocol.h"

enum TelemetryUnit {
  UNIT_RAW,
  UNIT_VOLTS,
  UNIT_AMPS,
  UNIT_MILLIAMPS,
  UNIT_KTS,
  UNIT_METERS_PER_SECOND,
  UNIT_FEET_PER_SECOND,
  UNIT_KMH,
  UNIT_SPEED = UNIT_KMH,
  UNIT_MPH,
  UNIT_METERS,
  UNIT_DIST = UNIT_METERS,
  UNIT_FEET,
  UNIT_CELSIUS,
  UNIT_TEMPERATURE = UNIT_CELSIUS,
  UNIT_FAHRENHEIT,
  UNIT_PERCENT,
  UNIT_MAH,
  UNIT_WATTS,
  UNIT_MILLIWATTS,
  UNIT_DB,
  UNIT_RPMS,
  UNIT_G,
  UNIT_DEGREE,
  UNIT_RADIANS,
  UNIT_MILLILITERS,
  UNIT_FLOZ,
  UNIT_MILLILITERS_PER_MINUTE,
  UNIT_HERTZ,
  UNIT_MS,
  UNIT_US,
  UNIT_KM,
  UNIT_DBM,
  UNIT_MAX = UNIT_DBM,
  UNIT_SPARE6,
  UNIT_SPARE7,
  UNIT_SPARE8,
  UNIT_SPARE9,
  UNIT_SPARE10,
  UNIT_HOURS,
  UNIT_MINUTES,
  UNIT_SECONDS,
  // FrSky format used for these fields, could be another format in the future
  UNIT_FIRST_VIRTUAL,
  UNIT_CELLS = UNIT_FIRST_VIRTUAL,
  UNIT_DATETIME,
  UNIT_GPS,
  UNIT_BITFIELD,
  UNIT_TEXT,
  // Internal units (not stored in sensor unit)
  UNIT_GPS_LONGITUDE,
  UNIT_GPS_LATITUDE,
  UNIT_DATETIME_YEAR,
  UNIT_DATETIME_DAY_MONTH,
  UNIT_DATETIME_HOUR_MIN,
  UNIT_DATETIME_SEC
};

struct CrossfireSensor {
  const uint8_t id;
  const uint8_t subId;
  const char * name;
  const TelemetryUnit unit;
  const uint8_t precision;
};

#define M_PIf       3.14159265358979323846f
#define RAD    (M_PIf / 180.0f)
#define RADIANS10000_TO_DEGREES(VALUE) (VALUE/(1000.0f*RAD*10))

typedef struct _crsf_telemtry_data_s
{
    // handled
    uint32_t last_update;
    bool telemetry_gotFix;
    bool telemetry_gotAlt;
    int32_t telemetry_lat;
    int32_t telemetry_lon;
    int16_t telemetry_alt;
    int16_t telemetry_sats;

    // not handle
    int32_t telemetry_time;
    int32_t telemetry_date;
    int16_t telemetry_age;

    uint8_t telemetry_failed_cs;

    int16_t telemetry_voltage;
    float telemetry_current;

    float telemetry_course;
    float telemetry_speed;
    float telemetry_declination;
    float telemetry_hdop;

    int16_t telemetry_pitch;
    int16_t telemetry_roll;
    int16_t telemetry_yaw;
    char telemtry_flightMode[5];

    char oled_screen[6][30];
    bool en_screen;

    void init()
    {
        last_update = 0;
        telemetry_gotFix = false;
        telemetry_gotAlt = false;
        telemetry_lat = 0;
        telemetry_lon = 0;
        telemetry_alt = 0;
        telemetry_sats = 0;

        // not handle
        telemetry_time = 0;
        telemetry_date = 0;
        telemetry_age = 0;

        telemetry_failed_cs = 0;

        telemetry_voltage = 0;
        telemetry_current = 0.0f;

        telemetry_course = 0.0f;
        telemetry_speed = 0.0f;
        telemetry_declination = 0.0f;
        telemetry_hdop = 0.0f;

        telemetry_pitch = 0;
        telemetry_roll = 0;
        telemetry_yaw = 0;
        
        memset(telemtry_flightMode, 0, sizeof(telemtry_flightMode));
        memset(oled_screen, 0, sizeof(oled_screen));
    }
    
    bool makeScreen(int page)
    {
      en_screen = false;
      switch (page)
      {
      case 0:
        // /* code */
        // break;
      default:
        sprintf(oled_screen[0], "Fix:%c Sta:%d Vol%d", telemetry_gotFix&&telemetry_gotAlt?'Y':'N',telemetry_sats, telemetry_voltage);
        sprintf(oled_screen[1], "Lat:%d ", telemetry_lat);
        sprintf(oled_screen[2], "Lon:%d ", telemetry_lon);
        sprintf(oled_screen[3], "Alt:%d Cur%.1f", telemetry_alt, telemetry_current);
        sprintf(oled_screen[4], "Pit:%.2f Rol:%.2f", RADIANS10000_TO_DEGREES(telemetry_pitch), RADIANS10000_TO_DEGREES(telemetry_roll));
        sprintf(oled_screen[5], "Yaw:%.2f", RADIANS10000_TO_DEGREES(telemetry_yaw));
        break;
      }
      en_screen = true;
      return true;
    }
} crsf_telemtry_data_s;

void crossfireProcessData(uint8_t nextPayloadSize, uint8_t *payloadData, uint32_t now);
