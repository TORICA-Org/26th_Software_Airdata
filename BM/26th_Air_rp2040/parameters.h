// 変数のextern宣言を書きます

#pragma once // インクルードガード（複数回読み込まれないようにする）

//ピン設定用
extern const int LED_ICS;
extern const int LED_Under;
extern const int LED_Air_pico;
extern const int LED_Air_xiao;

//その他変数
//SD送信用バッファ
extern char UART_SD[512]; //512文字まで
extern float const_platform_altitude_m;


//離陸・スピード判定
extern enum flight_phase;
extern enum speed_level;



//BNO055
extern volatile float data_air_bno_accx_mss;
extern volatile float data_air_bno_accy_mss;
extern volatile float data_air_bno_accz_mss;
extern volatile float data_air_bno_qw;
extern volatile float data_air_bno_qx;
extern volatile float data_air_bno_qy;
extern volatile float data_air_bno_qz;
extern volatile float data_air_bno_roll;
extern volatile float data_air_bno_pitch;
extern volatile float data_air_bno_yaw;

//BMP390
extern volatile float data_air_bmp_pressure_hPa;
extern volatile float data_air_bmp_temperature_deg;
extern volatile float data_air_bmp_altitude_m;

//GPS
extern volatile uint8_t data_air_gps_hour;
extern volatile uint8_t data_air_gps_minute;
extern volatile uint8_t data_air_gps_second;
extern volatile uint8_t data_air_gps_centisecond;
extern volatile double data_air_gps_latitude_deg;
extern volatile double data_air_gps_longitude_deg;
extern volatile double data_air_gps_altitude_m;
extern volatile double data_air_gps_groundspeed_ms;

//SDP31
extern volatile float data_air_sdp_differentialPressure_Pa;
extern volatile float data_air_sdp_airspeed_ms;

//AoA,AoS
extern volatile float data_air_AoA_angle_deg;
extern volatile float data_air_AoS_angle_deg;

//ICS基盤
extern volatile int data_ics_angle;


//Under電装部
extern volatile float data_under_bmp_pressure_hPa;
extern volatile float data_under_bmp_temperature_deg;
extern volatile float data_under_bmp_altitude_m;
extern volatile float data_under_urm_altitude_m;
extern volatile bool data_under_tsd20_istakeoff;





