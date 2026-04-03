/*---------------------------------------------------

このファイルの役割：変数，定数のextern宣言
最終更新：2026/02/20 19:29
更新内容：altitude_bmp_urm_offset_m.get()の追加

----------------------------------------------------*/

#pragma once  // インクルードガード（複数回読み込まれないようにする）


extern const float const_platform_altitude_m;

//離陸・スピード判定
enum FlightPhase {
  PLATFORM,
  HIGH_LEVEL,
  MID_LEVEL,
  LOW_LEVEL
};
extern FlightPhase flight_phase;

enum SpeedLevel {
  FAST,
  NORMAL,
  SLOW
};
extern SpeedLevel speed_level;

//離陸判定
extern volatile bool takeoff;


//動作時間
extern uint32_t time_ms;


//air_xiao向け　bico生存確認用
extern volatile bool air_bico_is_alive;


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

//以下はキャリブレーション状態保存用
extern volatile uint8_t data_air_bno_cal_system;
extern volatile uint8_t data_air_bno_cal_gyro;
extern volatile uint8_t data_air_bno_cal_accel;
extern volatile uint8_t data_air_bno_cal_mag;

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
extern bool under_is_alive;
extern volatile float data_under_bmp_pressure_hPa;
extern volatile float data_under_bmp_temperature_deg;
extern volatile float data_under_bmp_altitude_m;
extern volatile float data_under_urm_altitude_m;
extern volatile float data_under_tsd20_altitude_m;


//filtered
extern float estimated_altitude_lake_m;

//雑に実装．air_xiao用変数
extern float data_altitude_bmp_urm_offset_m;

// altitude_bmp_urm_offset_m: move-median buffer (defined in calculate_altitude.cpp)
#include <TORICA_MoveMedian.h>
extern TORICA_MoveMedian<400> altitude_bmp_urm_offset_m;
