#include <Arduino.h>
#include "TORICA_parameters.h"

//ピン宣言用
//Bico air_rp2040用
//LED設定
const int LED_ICS = 8;
const int LED_Under = 9;
const int LED_Air_pico = 10;
const int LED_Air_xiao = 11;
const int LED_GPS = 12;
const int LED_SD = 13;

//UARTピン設定
const int Serial_ICS_TX = 10;
const int Serial_ICS_RX = 11;
const int Serial_GPS_TX = 2;
const int Serial_GPS_RX = 3;
const int SerialAir_xiao_TX = 16;
const int SerialAir_xiao_RX = 17;
const int SerialUnder_TX = 4;
const int SerialUnder_RX = 5;

//I2Cピン設定
const int bico_I2C0_SDA = 20;
const int bico_I2C0_SCL = 21;
const int bico_I2C1_SDA = 26;
const int bico_I2C1_SCL = 27;


//SD送信用バッファ
char UART_SD[512] = { 0 };  //512文字まで

//UART送受信用バッファ
char readUART_BUF[512] = { 0 };
char sendUART_BUF[512] = { 0 };

const float const_platform_altitude_m = 10.0f;

//flight_phase, speed_level
FlightPhase flight_phase = PLATFORM;
SpeedLevel speed_level = SLOW;

//動作時間
uint32_t time_ms = 0;

//air_xiao向け　bico生存確認用
volatile bool air_bico_is_alive = false;

// BNO055
volatile float data_air_bno_accx_mss = 0.0f;
volatile float data_air_bno_accy_mss = 0.0f;
volatile float data_air_bno_accz_mss = 0.0f;
volatile float data_air_bno_qw = 0.0f;
volatile float data_air_bno_qx = 0.0f;
volatile float data_air_bno_qy = 0.0f;
volatile float data_air_bno_qz = 0.0f;
volatile float data_air_bno_roll = 0.0f;
volatile float data_air_bno_pitch = 0.0f;
volatile float data_air_bno_yaw = 0.0f;
//以下はキャリブレーション状態保存用
volatile uint8_t data_air_bno_cal_system = 0;
volatile uint8_t data_air_bno_cal_gyro = 0;
volatile uint8_t data_air_bno_cal_accel = 0;
volatile uint8_t data_air_bno_cal_mag = 0;

// BMP390
volatile float data_air_bmp_pressure_hPa = 0.0f;
volatile float data_air_bmp_temperature_deg = 0.0f;
volatile float data_air_bmp_altitude_m = 0.0f;

// GPS
volatile uint8_t data_air_gps_hour = 0;
volatile uint8_t data_air_gps_minute = 0;
volatile uint8_t data_air_gps_second = 0;
volatile uint8_t data_air_gps_centisecond = 0;
volatile double data_air_gps_latitude_deg = 0.0;
volatile double data_air_gps_longitude_deg = 0.0;
volatile double data_air_gps_altitude_m = 0.0;
volatile double data_air_gps_groundspeed_ms = 0.0;

// SDP31
volatile float data_air_sdp_differentialPressure_Pa = 0.0f;
volatile float data_air_sdp_airspeed_ms = 0.0f;

// AoA, AoS
volatile float data_air_AoA_angle_deg = 0.0f;
volatile float data_air_AoS_angle_deg = 0.0f;

// ICS
volatile int data_ics_angle = 0;

// Under用
bool under_is_alive = false;
volatile float data_under_bmp_pressure_hPa = 0.0f;
volatile float data_under_bmp_temperature_deg = 0.0f;
volatile float data_under_bmp_altitude_m = 0.0f;
volatile float data_under_urm_altitude_m = 0.0f;
volatile float data_under_tsd20_altitude_m = 0.0f;

// filtered
float estimated_altitude_lake_m = 0.0f;

//雑に実装．air_xiao用変数
float data_altitude_bmp_urm_offset_m = 0.0f;