// 変数の定義を書きます

#include <Arduino.h> // Arduinoの基本的な関数を使えるようにする
#include "parameters.h" // このファイルで定義する宣言が書かれたヘッダーファイル(.h)をインクルード

//ピン設定用
const int LED_ICS = 2;
const int LED_Under = 3;
const int LED_Air_pico = 4;
const int LED_Air_xiao = 5;



//離陸判定
enum {
    PLATFORM,
    HIGH_LEVEL,
    MID_LEVEL,
    LOW_LEVEL
} flight_phase = PLATFORM;

//スピードレベル
enum {
    FAST,
    NORMAL,
    SLOW
} speed_level = NORMAL;


//その他変数
//SD送信用バッファ
char UART_SD[512]; //512文字まで

//プラットフォームの高さ+機体の高さ(m)
const float const_platform_altitude_m = 10.6;


//data_場所_センサー名_データ種類_単位

//BNO055
volatile float data_air_bno_accx_mss = 0;
volatile float data_air_bno_accy_mss = 0;
volatile float data_air_bno_accz_mss = 0;
volatile float data_air_bno_qw = 0;
volatile float data_air_bno_qx = 0;
volatile float data_air_bno_qy = 0;
volatile float data_air_bno_qz = 0;
volatile float data_air_bno_roll = 0;
volatile float data_air_bno_pitch = 0;
volatile float data_air_bno_yaw = 0;

//BMP390
volatile float data_air_bmp_pressure_hPa = 0;
volatile float data_air_bmp_temperature_deg = 0;
volatile float data_air_bmp_altitude_m = 0;

//GPS
volatile uint8_t data_air_gps_hour = 0;
volatile uint8_t data_air_gps_minute = 0;
volatile uint8_t data_air_gps_second = 0;
volatile uint8_t data_air_gps_centisecond = 0;
volatile double data_air_gps_latitude_deg = 0;
volatile double data_air_gps_longitude_deg = 0;
volatile double data_air_gps_altitude_m = 0;
volatile double data_air_gps_groundspeed_ms = 0;

//SDP31
volatile float data_air_sdp_differentialPressure_Pa = 0;
volatile float data_air_sdp_airspeed_ms = 0;

//AoA,AoS
volatile float data_air_AoA_angle_deg = 0;
volatile float data_air_AoS_angle_deg = 0;

//ICS基盤
volatile int data_ics_angle = 0;


//Under電装部
volatile float data_under_bmp_pressure_hPa = 0;
volatile float data_under_bmp_temperature_deg = 0;
volatile float data_under_bmp_altitude_m = 0;
volatile float data_under_urm_altitude_m = 0;
volatile bool data_under_tsd20_istakeoff = false;


