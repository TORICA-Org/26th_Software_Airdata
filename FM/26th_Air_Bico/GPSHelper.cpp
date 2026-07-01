#include "GPSHelper.h"
#include <TinyGPSPlus.h>
#include "parameters.h"
#include "Bico_config.h"

SerialPIO Serial_GPS(Serial_GPS_TX, Serial_GPS_RX, 2048);

// TinyGPSPlusインスタンス化
TinyGPSPlus gps;

// TinyGPSPlusのカスタムコード．Teseo LIV3FLは$GNRMCの8番目のフィールドに進行方向を示す値を出力する．
TinyGPSCustom headingDeg(gps, "GNRMC", 8); 

void initGPS(){
    Serial_GPS.begin(460800, SERIAL_8N1); // GPSは8N1で通信
    Serial.print("GNSS Initialization Complete");
}

void read_gps(){
    while (Serial_GPS.available() > 0){
        if (gps.encode(Serial_GPS.read())){

            // 受信データを格納
            data_air_gps_hour = gps.time.hour();
            data_air_gps_minute = gps.time.minute();
            data_air_gps_second = gps.time.second();
            data_air_gps_centisecond = gps.time.centisecond();
            data_air_gps_latitude_deg = gps.location.lat();
            data_air_gps_longitude_deg = gps.location.lng();
            data_air_gps_altitude_m = gps.altitude.meters();
            data_air_gps_groundspeed_ms = gps.speed.kmph() * 1000 / 3600;
            data_air_gps_heading_deg = atof(headingDeg.value());
            data_air_gps_satellites = gps.satellites.value(); // 衛星補足数を取得

        }
    }
}


