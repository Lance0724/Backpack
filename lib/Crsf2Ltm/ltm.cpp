/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * LightTelemetry from KipK
 *
 * Minimal one way telemetry protocol for really low bitrates (1200/2400 bauds).
 * Effective for ground OSD, groundstation HUD and Antenna tracker
 * http://www.wedontneednasa.com/2014/02/lighttelemetry-v2-en-route-to-ground-osd.html
 *
 * This implementation is for LTM v2 > 2400 baud rates
 *
 * Cleanflight implementation by Jonathan Hudson
 */

#include "ltm.h"
#include "crossfire.h"

#define TELEMETRY_LTM_INITIAL_PORT_MODE MODE_TX
#define LTM_CYCLETIME   100
#define LTM_SCHEDULE_SIZE (1000/LTM_CYCLETIME)

extern crsf_telemetry_data_s crsf_tlm_data;

// static serialPort_t *ltmPort;
// static serialPortConfig_t *portConfig;
// static bool ltmEnabled;
// static portSharing_e ltmPortSharing;
// static uint8_t ltmFrame[LTM_MAX_MESSAGE_SIZE];
// static uint8_t ltm_x_counter;

// static void ltm_initialise_packet(sbuf_t *dst)
// {
//     dst->ptr = ltmFrame;
//     dst->end = ARRAYEND(ltmFrame);

//     sbufWriteU8(dst, '$');
//     sbufWriteU8(dst, 'T');
// }

// static void ltm_finalise(sbuf_t *dst)
// {
//     uint8_t crc = 0;
//     for (const uint8_t *ptr = &ltmFrame[3]; ptr < dst->ptr; ++ptr) {
//         crc ^= *ptr;
//     }
//     sbufWriteU8(dst, crc);
//     sbufSwitchToReader(dst, ltmFrame);
//     // serialWriteBuf(ltmPort, sbufPtr(dst), sbufBytesRemaining(dst));
// }

/*
 * GPS G-frame 5Hhz at > 2400 baud
 * LAT LON SPD ALT SAT/FIX
 */
void ltm_gframe(sbuf_t *dst)
{
    uint8_t gps_fix_type = 0;
    int32_t ltm_lat = 0, ltm_lon = 0, ltm_alt = 0, ltm_gs = 0;

    if (!crsf_tlm_data.telemetry_gotAlt 
        && !crsf_tlm_data.telemetry_gotFix)
        gps_fix_type = 1;
    else if (crsf_tlm_data.telemetry_gotAlt  
            || crsf_tlm_data.telemetry_gotFix )
        gps_fix_type = 2;
    else if (crsf_tlm_data.telemetry_gotAlt 
            && crsf_tlm_data.telemetry_gotFix )
        gps_fix_type = 3;

    ltm_lat = crsf_tlm_data.telemetry_lat;
    ltm_lon = crsf_tlm_data.telemetry_lon;

    // (m/h +50m/h)/100  ->  m/s
    ltm_gs = (crsf_tlm_data.telemetry_speed * 100 - 50)/3600;

    ltm_alt = crsf_tlm_data.telemetry_alt * 100; // m to cm

    sbufWriteU8(dst, 'G');
    sbufWriteU32(dst, ltm_lat * 10);
    sbufWriteU32(dst, ltm_lon * 10);
    sbufWriteU8(dst, (uint8_t)ltm_gs);
    sbufWriteU32(dst, ltm_alt);
    sbufWriteU8(dst, (crsf_tlm_data.telemetry_sats << 2) | gps_fix_type);
}

/*
 * Sensors S-frame 5Hhz at > 2400 baud
 * VBAT(mv)  Current(ma)   RSSI  AIRSPEED  ARM/FS/FMOD
 * Flight mode(0-19):
 *     0: Manual, 1: Rate, 2: Attitude/Angle, 3: Horizon,
 *     4: Acro, 5: Stabilized1, 6: Stabilized2, 7: Stabilized3,
 *     8: Altitude Hold, 9: Loiter/GPS Hold, 10: Auto/Waypoints,
 *     11: Heading Hold / headFree, 12: Circle, 13: RTH, 14: FollowMe,
 *     15: LAND, 16:FlybyWireA, 17: FlybywireB, 18: Cruise, 19: Unknown
 */

// void ltm_sframe(sbuf_t *dst)
// {
//     ltm_modes_e lt_flightmode;

//     if (FLIGHT_MODE(MANUAL_MODE))
//         lt_flightmode = LTM_MODE_MANUAL;
//     else if (FLIGHT_MODE(NAV_WP_MODE))
//         lt_flightmode = LTM_MODE_WAYPOINTS;
//     else if (FLIGHT_MODE(NAV_RTH_MODE))
//         lt_flightmode = LTM_MODE_RTH;
//     else if (FLIGHT_MODE(NAV_POSHOLD_MODE))
//         lt_flightmode = LTM_MODE_GPSHOLD;
//     else if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE))
//         lt_flightmode = LTM_MODE_CRUISE;
//     else if (FLIGHT_MODE(NAV_LAUNCH_MODE))
//         lt_flightmode = LTM_MODE_LAUNCH;
//     else if (FLIGHT_MODE(AUTO_TUNE))
//         lt_flightmode = LTM_MODE_AUTOTUNE;
//     else if (FLIGHT_MODE(NAV_ALTHOLD_MODE))
//         lt_flightmode = LTM_MODE_ALTHOLD;
//     else if (FLIGHT_MODE(HEADFREE_MODE) || FLIGHT_MODE(HEADING_MODE))
//         lt_flightmode = LTM_MODE_HEADHOLD;
//     else if (FLIGHT_MODE(ANGLE_MODE))
//         lt_flightmode = LTM_MODE_ANGLE;
//     else if (FLIGHT_MODE(HORIZON_MODE))
//         lt_flightmode = LTM_MODE_HORIZON;
//     else
//         lt_flightmode = LTM_MODE_RATE;      // Rate mode

//     uint8_t lt_statemode = (ARMING_FLAG(ARMED)) ? 1 : 0;
//     if (failsafeIsActive())
//         lt_statemode |= 2;
//     sbufWriteU8(dst, 'S');
//     sbufWriteU16(dst, getBatteryVoltage() * 10);    //vbat converted to mv
//     sbufWriteU16(dst, (uint16_t)constrain(getMAhDrawn(), 0, 0xFFFF));    // current mAh (65535 mAh max)
//     sbufWriteU8(dst, (uint8_t)((getRSSI() * 254) / 1023));        // scaled RSSI (uchar)
// #if defined(USE_PITOT)
//     sbufWriteU8(dst, sensors(SENSOR_PITOT) ? pitot.airSpeed / 100.0f : 0);  // in m/s
// #else
//     sbufWriteU8(dst, 0);
// #endif
//     sbufWriteU8(dst, (lt_flightmode << 2) | lt_statemode);
// }
/*
 * Attitude A-frame - 10 Hz at > 2400 baud
 *  PITCH ROLL HEADING
 */
void ltm_aframe(sbuf_t *dst)
{
    sbufWriteU8(dst, 'A');
    sbufWriteU16(dst, RADIANS10000_TO_DEGREES(crsf_tlm_data.telemetry_pitch));
    sbufWriteU16(dst, RADIANS10000_TO_DEGREES(crsf_tlm_data.telemetry_roll));
    sbufWriteU16(dst, RADIANS10000_TO_DEGREES(crsf_tlm_data.telemetry_yaw));
}

// #if defined(USE_GPS)
// /*
//  * OSD additional data frame, 1 Hz rate
//  *  This frame will be ignored by Ghettostation, but processed by GhettOSD if it is used as standalone onboard OSD
//  *  home pos, home alt, direction to home
//  */
// void ltm_oframe(sbuf_t *dst)
// {
//     sbufWriteU8(dst, 'O');
//     sbufWriteU32(dst, GPS_home.lat);
//     sbufWriteU32(dst, GPS_home.lon);
//     sbufWriteU32(dst, GPS_home.alt);
//     sbufWriteU8(dst, 1);                 // OSD always ON
//     sbufWriteU8(dst, STATE(GPS_FIX_HOME) ? 1 : 0);
// }
// #endif

// /*
//  * Extended information data frame, 1 Hz rate
//  *  This frame is intended to report extended GPS and NAV data, however at the moment it contains only HDOP value
//  */
// void ltm_xframe(sbuf_t *dst)
// {
//     uint8_t sensorStatus =
//         (isHardwareHealthy() ? 0 : 1) << 0;     // bit 0 - hardware failure indication (1 - something is wrong with the hardware sensors)

//     sbufWriteU8(dst, 'X');
// #if defined(USE_GPS)
//     sbufWriteU16(dst, gpsSol.hdop);
// #else
//     sbufWriteU16(dst, 9999);
// #endif
//     sbufWriteU8(dst, sensorStatus);
//     sbufWriteU8(dst, ltm_x_counter);
//     sbufWriteU8(dst, getDisarmReason());
//     sbufWriteU8(dst, 0);
//     ltm_x_counter++; // overflow is OK
// }

/** OSD additional data frame, ~4 Hz rate, navigation system status
 */
// void ltm_nframe(sbuf_t *dst)
// {
//     sbufWriteU8(dst, 'N');
//     sbufWriteU8(dst, NAV_Status.mode);
//     sbufWriteU8(dst, NAV_Status.state);
//     sbufWriteU8(dst, NAV_Status.activeWpAction);
//     sbufWriteU8(dst, NAV_Status.activeWpNumber);
//     sbufWriteU8(dst, NAV_Status.error);
//     sbufWriteU8(dst, NAV_Status.flags);
// }

#define LTM_BIT_AFRAME  (1 << 0)
#define LTM_BIT_GFRAME  (1 << 1)
#define LTM_BIT_SFRAME  (1 << 2)
#define LTM_BIT_OFRAME  (1 << 3)
#define LTM_BIT_NFRAME  (1 << 4)
#define LTM_BIT_XFRAME  (1 << 5)

/*
 * This is the normal (default) scheduler, needs c. 4800 baud or faster
 * Equates to c. 303 bytes / second
 */
// static uint8_t ltm_normal_schedule[LTM_SCHEDULE_SIZE] = {
//     LTM_BIT_AFRAME | LTM_BIT_GFRAME,
//     LTM_BIT_AFRAME | LTM_BIT_SFRAME | LTM_BIT_OFRAME,
//     LTM_BIT_AFRAME | LTM_BIT_GFRAME,
//     LTM_BIT_AFRAME | LTM_BIT_SFRAME | LTM_BIT_NFRAME,
//     LTM_BIT_AFRAME | LTM_BIT_GFRAME,
//     LTM_BIT_AFRAME | LTM_BIT_SFRAME | LTM_BIT_XFRAME,
//     LTM_BIT_AFRAME | LTM_BIT_GFRAME,
//     LTM_BIT_AFRAME | LTM_BIT_SFRAME | LTM_BIT_NFRAME,
//     LTM_BIT_AFRAME | LTM_BIT_GFRAME,
//     LTM_BIT_AFRAME | LTM_BIT_SFRAME | LTM_BIT_NFRAME
// };

/*
 * This is the medium scheduler, needs c. 2400 baud or faster
 * Equates to c. 164 bytes / second
 */
// static uint8_t ltm_medium_schedule[LTM_SCHEDULE_SIZE] = {
//     LTM_BIT_AFRAME,
//     LTM_BIT_GFRAME,
//     LTM_BIT_AFRAME | LTM_BIT_SFRAME,
//     LTM_BIT_OFRAME,
//     LTM_BIT_AFRAME | LTM_BIT_XFRAME,
//     LTM_BIT_OFRAME,
//     LTM_BIT_AFRAME | LTM_BIT_SFRAME,
//     LTM_BIT_GFRAME,
//     LTM_BIT_AFRAME,
//     LTM_BIT_NFRAME
// };

/*
 * This is the slow scheduler, needs c. 1200 baud or faster
 * Equates to c. 105 bytes / second (91 b/s if the second GFRAME is zeroed)
 */
// static uint8_t ltm_slow_schedule[LTM_SCHEDULE_SIZE] = {
//     LTM_BIT_GFRAME,
//     LTM_BIT_SFRAME,
//     LTM_BIT_AFRAME,
//     0,
//     LTM_BIT_OFRAME,
//     LTM_BIT_XFRAME,
//     LTM_BIT_GFRAME, // consider zeroing this for even lower bytes/sec
//     0,
//     LTM_BIT_AFRAME,
//     LTM_BIT_NFRAME,
// };

// /* Set by initialisation */
// static uint8_t *ltm_schedule;

// static void process_ltm(void)
// {
//     static uint8_t ltm_scheduler = 0;
//     uint8_t current_schedule = ltm_schedule[ltm_scheduler];

//     sbuf_t ltmFrameBuf;
//     sbuf_t *dst = &ltmFrameBuf;

//     if (current_schedule & LTM_BIT_AFRAME) {
//         ltm_initialise_packet(dst);
//         ltm_aframe(dst);
//         ltm_finalise(dst);
//     }

// #if defined(USE_GPS)
//     if (current_schedule & LTM_BIT_GFRAME) {
//         ltm_initialise_packet(dst);
//         ltm_gframe(dst);
//         ltm_finalise(dst);
//     }

//     if (current_schedule & LTM_BIT_OFRAME) {
//         ltm_initialise_packet(dst);
//         ltm_oframe(dst);
//         ltm_finalise(dst);
//     }

//     if (current_schedule & LTM_BIT_XFRAME) {
//         ltm_initialise_packet(dst);
//         ltm_xframe(dst);
//         ltm_finalise(dst);
//     }
// #endif

//     if (current_schedule & LTM_BIT_SFRAME) {
//         ltm_initialise_packet(dst);
//         ltm_sframe(dst);
//         ltm_finalise(dst);
//     }

//     if (current_schedule & LTM_BIT_NFRAME) {
//         ltm_initialise_packet(dst);
//         ltm_nframe(dst);
//         ltm_finalise(dst);
//     }

//     ltm_scheduler = (ltm_scheduler + 1) % 10;
// }

// void handleLtmTelemetry(void)
// {
//     static uint32_t ltm_lastCycleTime;
//     if (!ltmEnabled)
//         return;
//     if (!ltmPort)
//         return;
//     const uint32_t now = millis();
//     if ((now - ltm_lastCycleTime) >= LTM_CYCLETIME) {
//         process_ltm();
//         ltm_lastCycleTime = now;
//     }
// }

// void freeLtmTelemetryPort(void)
// {
//     closeSerialPort(ltmPort);
//     ltmPort = NULL;
//     ltmEnabled = false;
// }

// void initLtmTelemetry(void)
// {
//     portConfig = findSerialPortConfig(FUNCTION_TELEMETRY_LTM);
//     ltmPortSharing = determinePortSharing(portConfig, FUNCTION_TELEMETRY_LTM);
// }

// static void configureLtmScheduler(void)
// {

//     /* setup scheduler, default to 'normal' */
//     if (telemetryConfig()->ltmUpdateRate == LTM_RATE_MEDIUM)
//         ltm_schedule = ltm_medium_schedule;
//     else if (telemetryConfig()->ltmUpdateRate == LTM_RATE_SLOW)
//         ltm_schedule = ltm_slow_schedule;
//     else
//         ltm_schedule = ltm_normal_schedule;

// }

// void configureLtmTelemetryPort(void)
// {
//     if (!portConfig) {
//         return;
//     }
//     baudRate_e baudRateIndex = portConfig->telemetry_baudrateIndex;
//     if (baudRateIndex == BAUD_AUTO) {
//         baudRateIndex = BAUD_19200;
//     }

//     /* Sanity check that we can support the scheduler */
//     if (baudRateIndex == BAUD_2400 && telemetryConfig()->ltmUpdateRate == LTM_RATE_NORMAL)
//          ltm_schedule = ltm_medium_schedule;
//     if (baudRateIndex == BAUD_1200)
//          ltm_schedule = ltm_slow_schedule;

//     ltmPort = openSerialPort(portConfig->identifier, FUNCTION_TELEMETRY_LTM, NULL, NULL, baudRates[baudRateIndex], TELEMETRY_LTM_INITIAL_PORT_MODE, SERIAL_NOT_INVERTED);
//     if (!ltmPort)
//         return;
//     ltm_x_counter = 0;
//     ltmEnabled = true;
// }

// void checkLtmTelemetryState(void)
// {
//     if (portConfig && telemetryCheckRxPortShared(portConfig)) {
//         if (!ltmEnabled && telemetrySharedPort != NULL) {
//             ltmPort = telemetrySharedPort;
//             configureLtmScheduler();
//             ltmEnabled = true;
//         }
//     } else {
//         bool newTelemetryEnabledValue = telemetryDetermineEnabledState(ltmPortSharing);
//         if (newTelemetryEnabledValue == ltmEnabled)
//             return;
//         if (newTelemetryEnabledValue){
//             configureLtmScheduler();
//             configureLtmTelemetryPort();

//     }
//         else
//             freeLtmTelemetryPort();
//     }
// }

int getLtmFrame(uint8_t *frame, ltm_frame_e ltmFrameType)
{
    static uint8_t ltmFrame[LTM_MAX_MESSAGE_SIZE];

    sbuf_t ltmFrameBuf = { .ptr = ltmFrame, .end =ARRAYEND(ltmFrame) };
    sbuf_t * const sbuf = &ltmFrameBuf;

    sbufWriteU8(sbuf, '$');
    sbufWriteU8(sbuf, 'T');

    switch (ltmFrameType) {
    default:
    case LTM_AFRAME:
        ltm_aframe(sbuf);
        break;
    // case LTM_SFRAME:
    //     ltm_sframe(sbuf);
    //     break;
    case LTM_GFRAME:
        ltm_gframe(sbuf);
        break;
    // case LTM_OFRAME:
    //     ltm_oframe(sbuf);
    //     break;
    // case LTM_XFRAME:
    //     ltm_xframe(sbuf);
    //     break;
    // case LTM_NFRAME:
    //     ltm_nframe(sbuf);
    //     break;
    }
    
    uint8_t crc = 0;
    for (const uint8_t *ptr = &ltmFrame[3]; ptr < sbuf->ptr; ++ptr) {
        crc ^= *ptr;
    }
    sbufWriteU8(sbuf, crc);
    
    sbufSwitchToReader(sbuf, ltmFrame);
    const int frameSize = sbufBytesRemaining(sbuf);
    for (int ii = 0; sbufBytesRemaining(sbuf); ++ii) {
        frame[ii] = sbufReadU8(sbuf);
    }
    return frameSize;
}
