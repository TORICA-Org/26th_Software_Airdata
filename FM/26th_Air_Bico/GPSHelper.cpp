#include "GPSHelper.h"
#include <TinyGPSPlus.h>
#include "parameters.h"
#include "Bico_config.h"

// SerialPIO Serial_GPS(Serial_GPS_TX, Serial_GPS_RX, 4096);
SerialPIO Serial_GPS(14, 13, 4096);

// TinyGPSPlusインスタンス化
TinyGPSPlus gps;

void initGPS(){
    Serial_GPS.begin(115200);
    Serial.print("GNSS Initialization Complete");
}

void read_gps(){

    // for debug
    static unsigned long success_count = 0;

    while (Serial_GPS.available() > 0){
        char c = Serial_GPS.read();
        if (gps.encode(c)){

            // 受信データを格納
            data_air_gps_hour = gps.time.hour();
            data_air_gps_minute = gps.time.minute();
            data_air_gps_second = gps.time.second();
            data_air_gps_centisecond = gps.time.centisecond();
            data_air_gps_latitude_deg = gps.location.lat();
            data_air_gps_longitude_deg = gps.location.lng();
            data_air_gps_altitude_m = gps.altitude.meters();
            data_air_gps_groundspeed_ms = gps.speed.kmph() * 1000 / 3600;
            data_air_gps_heading_deg = gps.course.deg();
            data_air_gps_satellites = gps.satellites.value(); // 衛星補足数を取得

        }
    }
}


// void read_gps(){
//     // 診断用のカウンタ（staticで値を保持）
//     static unsigned long total_bytes = 0;
//     static unsigned long dollar_count = 0;
//     static unsigned long newline_count = 0;
//     static unsigned long last_report_time = 0;

//     // 10msの間に溜まった文字をパース
//     while (Serial_GPS.available() > 0){
//         char c = Serial_GPS.read();
//         total_bytes++; // 届いた総バイト数
        
//         if (c == '$')  dollar_count++;  // NMEAセンテンスの開始合図
//         if (c == '\n') newline_count++; // NMEAセンテンスの終了合図

//         // TinyGPS++ に流し込む
//         if (gps.encode(c)){
//             // パース成功時の代入処理
//             data_air_gps_hour           = gps.time.hour();
//             data_air_gps_minute         = gps.time.minute();
//             data_air_gps_second         = gps.time.second();
//             data_air_gps_centisecond    = gps.time.centisecond();
//             data_air_gps_latitude_deg   = gps.location.lat();
//             data_air_gps_longitude_deg  = gps.location.lng();
//             data_air_gps_altitude_m     = gps.altitude.meters();
//             data_air_gps_groundspeed_ms = gps.speed.kmph() * 1000 / 3600;
//             data_air_gps_heading_deg    = gps.course.deg();
//             data_air_gps_satellites     = gps.satellites.value(); 
//         }
//     }

//     // 10msループの邪魔をしないよう、1秒に1回だけまとめてシリアルモニターに出力
//     if (millis() - last_report_time >= 1000) {
//         last_report_time = millis();

//         Serial.println("\n--- GNSS Realtime Status Report ---");
//         Serial.print("1. Received Chars (per sec): "); Serial.println(total_bytes);
//         Serial.print("2. Sentence Starts ($)     : "); Serial.println(dollar_count);
//         Serial.print("3. Sentence Ends (\\n)      : "); Serial.println(newline_count);
//         Serial.print("4. TinyGPS++ Passed Lines  : "); Serial.println(gps.passedChecksum());
//         Serial.print("5. TinyGPS++ Failed Lines  : "); Serial.println(gps.failedChecksum());
//         Serial.println("-----------------------------------");

//         // カウンタをリセット
//         total_bytes = 0;
//         dollar_count = 0;
//         newline_count = 0;
//     }
// }


