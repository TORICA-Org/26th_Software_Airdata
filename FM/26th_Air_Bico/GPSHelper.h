#pragma once
#include <Arduino.h>

struct GPSData {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t centisecond;
    double latitude_deg;
    double longitude_deg;
    double altitude_m;
    double groundspeed_ms;
    float heading_deg;
    uint8_t satellites;
};

void initGPS(int tx_pin = 14, int rx_pin = 13);
bool read_gps(GPSData& outData);
