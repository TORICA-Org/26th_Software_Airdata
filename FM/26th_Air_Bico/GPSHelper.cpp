#include "GPSHelper.h"
#include <TinyGPSPlus.h>

static SerialPIO* gpsSerial = nullptr;
static TinyGPSPlus gps;

void initGPS(int tx_pin, int rx_pin) {
    if (gpsSerial == nullptr) {
        gpsSerial = new SerialPIO(tx_pin, rx_pin, 4096);
    }
    gpsSerial->begin(115200);
    #ifdef DEBUG_MODE
    Serial.println("GNSS Initialization Complete");
    #endif
}

bool read_gps(GPSData& outData) {
    if (gpsSerial == nullptr) return false;

    bool encoded = false;

    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        if (gps.encode(c)) {
            outData.hour = gps.time.hour();
            outData.minute = gps.time.minute();
            outData.second = gps.time.second();
            outData.centisecond = gps.time.centisecond();
            outData.latitude_deg = gps.location.lat();
            outData.longitude_deg = gps.location.lng();
            outData.altitude_m = gps.altitude.meters();
            outData.groundspeed_ms = gps.speed.kmph() * 1000.0 / 3600.0;
            outData.heading_deg = gps.course.deg();
            outData.satellites = gps.satellites.value();
            encoded = true;
        }
    }
    return encoded;
}


