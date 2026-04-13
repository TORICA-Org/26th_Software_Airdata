/*------------------------------------

このファイルの役割：変数，定数の宣言
最終更新：2026/04/11 00:35
更新内容：胴体桁電装用変数の追加
注) 各基板において不要な変数も宣言されているが，メモリを余計に消費するため，不要なものはコメントアウトすること．

------------------------------------*/



#include <Arduino.h>
#include "parameters.h"


const float const_platform_altitude_m = 10.6f;

//flight_phase, speed_level
FlightPhase flight_phase = PLATFORM;
SpeedLevel speed_level = SLOW;

//離陸判定
volatile bool takeoff = false;

//動作時間
volatile uint32_t time_ms = 0;

// filtered
volatile float estimated_altitude_lake_m = const_platform_altitude_m;

//雑に実装．air_xiao用変数
volatile float data_altitude_bmp_urm_offset_m = 0.0f;


/*--- エアデータ用 ---*/

//BMP390
volatile float data_air_bmp_pressure_hPa = 0.0f;
volatile float data_air_bmp_temperature_deg = 0.0f;
volatile float data_air_bmp_altitude_m = 0.0f;

//GPS
volatile uint8_t data_air_gps_hour = 0;
volatile uint8_t data_air_gps_minute = 0;
volatile uint8_t data_air_gps_second = 0;
volatile uint8_t data_air_gps_centisecond = 0;
volatile double data_air_gps_latitude_deg = 0.0;
volatile double data_air_gps_longitude_deg = 0.0;
volatile double data_air_gps_altitude_m = 0.0;
volatile double data_air_gps_groundspeed_ms = 0.0;

//SDP31
volatile float data_air_sdp_differentialPressure_Pa = 0.0f;
volatile float data_air_sdp_airspeed_ms = 0.0f;

//AoA,AoS
volatile float data_air_AoA_angle_deg = 0.0f;
volatile float data_air_AoS_angle_deg = 0.0f;

//ICS基盤
volatile int data_ics_angle = 0;


/*--- 胴体桁電装用 ---*/
volatile bool psd_is_alive = false;

//BNO055
volatile float data_psd_bno_accx_mss = 0.0f;
volatile float data_psd_bno_accy_mss = 0.0f;
volatile float data_psd_bno_accz_mss = 0.0f;
volatile float data_psd_bno_qw = 0.0f;
volatile float data_psd_bno_qx = 0.0f;
volatile float data_psd_bno_qy = 0.0f;
volatile float data_psd_bno_qz = 0.0f;
volatile float data_psd_bno_roll = 0.0f;
volatile float data_psd_bno_pitch = 0.0f;
volatile float data_psd_bno_yaw = 0.0f;

//BNO055キャリブレーション状態保存用
volatile uint8_t data_psd_bno_cal_system = 0;
volatile uint8_t data_psd_bno_cal_gyro = 0;
volatile uint8_t data_psd_bno_cal_accel = 0;
volatile uint8_t data_psd_bno_cal_mag = 0;

//BMP390
volatile float data_psd_bmp_pressure_hPa = 0.0f;
volatile float data_psd_bmp_temperature_deg = 0.0f;
volatile float data_psd_bmp_altitude_m = 0.0f;



/*--- Under電装部 ---*/
volatile bool under_is_alive = false;
volatile float data_under_bmp_pressure_hPa = 0.0f;
volatile float data_under_bmp_temperature_deg = 0.0f;
volatile float data_under_bmp_altitude_m = 0.0f;
volatile float data_under_urm_altitude_m = 0.0f;
volatile float data_under_tsd20_altitude_m = 0.0f;