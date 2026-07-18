#include "sensor_reader.h"
#include "parameters.h"
#include "BMP3xx.h"
#include "AS5600.h"
#include "SDP31.h"
#include "GPSHelper.h"

void update_air_bmp() {
    BmpData bmp = read_bmp();
    data_air_bmp_pressure_hPa = bmp.pressure_hPa;
    data_air_bmp_temperature_deg = bmp.temperature_deg;
}

void update_air_AS5600() {
    AS5600Data angles = read_AS5600();
    data_air_AoA_angle_deg = angles.AoA_deg;
    data_air_AoS_angle_deg = angles.AoS_deg;
}

void update_air_SDP() {
    data_air_sdp_differentialPressure_Pa = read_SDP();
}

void update_air_gps() {
    GPSData gpsVal;
    if (read_gps(gpsVal)) {
        data_air_gps_hour = gpsVal.hour;
        data_air_gps_minute = gpsVal.minute;
        data_air_gps_second = gpsVal.second;
        data_air_gps_centisecond = gpsVal.centisecond;
        data_air_gps_latitude_deg = gpsVal.latitude_deg;
        data_air_gps_longitude_deg = gpsVal.longitude_deg;
        data_air_gps_altitude_m = gpsVal.altitude_m;
        data_air_gps_groundspeed_ms = gpsVal.groundspeed_ms;
        data_air_gps_heading_deg = gpsVal.heading_deg;
        data_air_gps_satellites = gpsVal.satellites;
    }
}
