#include "UARTHelper_air_xiao.h"
#include "parameters.h"
#include "Air_xiao_config.h"
#include <Arduino.h>

const uint8_t BICO_DATA_NUM = 54;


// TORICA_UARTインスタンス化
#include <TORICA_UART.h>
TORICA_UART Bico_UART(&Serial1);

char recv_buff[512];  // 受信する文字列を保存するためのバッファ

void initUART() {

  // UART初期化（<-まだ通信の開始処理はされていない）
  Serial1.setRxBufferSize(8192);  // バッファ(受信したデータの一時保管場所)サイズ指定(8192byte)

  // パラメータ設定とともに通信を開始
  // ICS通信の仕様に合わせ，`SERIAL_8E1`としている．
  // `8`:データビットの長さ
  // `E`:偶数パリティ(`N`:パリティなし，`O`:奇数パリティ)
  // `1`:ストップビット(データフレームの終わりを示すビット)の長さ
  Serial1.begin(460800, SERIAL_8E1, BICO_UART_RX, BICO_UART_TX);

}


// 以下UARTデータ受信処理

static unsigned long int last_Bico_time_ms = 0;
char readUART_BUF[1024] = {0};
const int readUART_BUF_SIZE = 1024;
bool Bico_is_alive = false;    // Bicoが生きているかどうか

// この関数はUARTを受信してreadUART_BUFにデータを格納するところまでを行う
void receiveLog (){

  // readUART_BUF の中身をすべて 0 で埋める（初期化）
  memset(readUART_BUF, 0, sizeof(readUART_BUF));

  if (Bico_UART.listenUART()) {
    strcpy(readUART_BUF, Bico_UART.buff);  // Bico_UART.buffの内容をreadUART_BUFにコピー
    Bico_is_alive = true; // データを受信したのでBicoは生きている
    last_Bico_time_ms = millis(); // 最終受信時間を更新
  } else {
    if (millis() - last_Bico_time_ms > 1000) {
      Bico_is_alive = false; // 1秒以上データが途絶えたらBicoは死んでいるとみなす
    } else {
      Bico_is_alive = true;
    }
  }
}



// ----------------------------------------------------
// 受信データを構造体に代入する
// ----------------------------------------------------
LogData convertArrayToLogData(const float* uart_data) {
  LogData data;

  // 1回目の受信 13個
  data.time_ms = uart_data[0];
  data.takeoff = uart_data[1];
  data.urm_is_reliable = static_cast<bool>(uart_data[2]);
  data.data_air_gps_hour = uart_data[3];
  data.data_air_gps_minute = uart_data[4];
  data.data_air_gps_second = uart_data[5];
  data.data_air_gps_centisecond = uart_data[6];
  data.data_air_gps_latitude_deg = uart_data[7];
  data.data_air_gps_longitude_deg = uart_data[8];
  data.data_air_gps_altitude_m = uart_data[9];
  data.data_air_gps_groundspeed_ms = uart_data[10];
  data.data_air_gps_heading_deg = uart_data[11];
  data.data_air_gps_satellites = uart_data[12];

  // 2回目の受信 11個
  data.filtered_bmp_altitude_m = uart_data[13];
  data.filtered_urm_altitude_m = uart_data[14];
  data.filtered_airspeed_ms = uart_data[15];
  data.data_air_bmp_pressure_hPa = uart_data[16];
  data.data_air_bmp_temperature_deg = uart_data[17];
  data.data_air_bmp_altitude_m = uart_data[18];
  data.data_air_sdp_differentialPressure_Pa = uart_data[19];
  data.data_air_sdp_airspeed_ms = uart_data[20];
  data.data_air_AoA_angle_deg = uart_data[21];
  data.data_air_AoS_angle_deg = uart_data[22];
  data.data_ics_angle = uart_data[23];

  // 3回目の受信 14個
  data.fslg_is_alive = static_cast<bool>(uart_data[24]);
  data.data_fslg_bno_qw = uart_data[25];
  data.data_fslg_bno_qx = uart_data[26];
  data.data_fslg_bno_qy = uart_data[27];
  data.data_fslg_bno_qz = uart_data[28];
  data.data_fslg_bno_roll = uart_data[29];
  data.data_fslg_bno_pitch = uart_data[30];
  data.data_fslg_bno_yaw = uart_data[31];
  data.data_fslg_lsm_roll = uart_data[32];
  data.data_fslg_lsm_pitch = uart_data[33];
  data.data_fslg_lsm_yaw = uart_data[34];
  data.data_fslg_bmp_pressure_hPa = uart_data[35];
  data.data_fslg_bmp_temperature_deg = uart_data[36];
  data.data_fslg_bmp_altitude_m = uart_data[37];

  // 4回目の受信 16個
  data.data_fslg_bno_accx_mss = uart_data[38];
  data.data_fslg_bno_accy_mss = uart_data[39];
  data.data_fslg_bno_accz_mss = uart_data[40];
  data.data_fslg_lsm_accx_mss = uart_data[41];
  data.data_fslg_lsm_accy_mss = uart_data[42];
  data.data_fslg_lsm_accz_mss = uart_data[43];
  data.data_fslg_bno_cal_system = static_cast<uint8_t>(uart_data[44]);
  data.data_fslg_bno_cal_gyro = static_cast<uint8_t>(uart_data[45]);
  data.data_fslg_bno_cal_accel = static_cast<uint8_t>(uart_data[46]);
  data.data_fslg_bno_cal_mag = static_cast<uint8_t>(uart_data[47]);
  data.under_is_alive = static_cast<bool>(uart_data[48]);
  data.data_under_bmp_pressure_hPa = uart_data[49];
  data.data_under_bmp_temperature_deg = uart_data[50];
  data.data_under_bmp_altitude_m = uart_data[51];
  data.data_under_urm_altitude_m = uart_data[52];
  data.data_under_tsd20_altitude_m = uart_data[53];

  return data;
}

// ----------------------------------------------------
// 構造体からグローバル変数へ代入
// ----------------------------------------------------
void applyLogDataToGlobals(const LogData& data) {
  // 離陸判定
  takeoff = data.takeoff;
  time_ms = data.time_ms;
  urm_is_reliable = data.urm_is_reliable;

  // エアデータ用
  data_air_gps_hour = data.data_air_gps_hour;
  data_air_gps_minute = data.data_air_gps_minute;
  data_air_gps_second = data.data_air_gps_second;
  data_air_gps_centisecond = data.data_air_gps_centisecond;
  data_air_gps_latitude_deg = data.data_air_gps_latitude_deg;
  data_air_gps_longitude_deg = data.data_air_gps_longitude_deg;
  data_air_gps_altitude_m = data.data_air_gps_altitude_m;
  data_air_gps_groundspeed_ms = data.data_air_gps_groundspeed_ms;
  data_air_gps_heading_deg = data.data_air_gps_heading_deg;
  data_air_gps_satellites = data.data_air_gps_satellites;

  filtered_bmp_altitude_m = data.filtered_bmp_altitude_m;
  filtered_urm_altitude_m = data.filtered_urm_altitude_m;
  filtered_airspeed_ms = data.filtered_airspeed_ms;
  data_air_bmp_pressure_hPa = data.data_air_bmp_pressure_hPa;
  data_air_bmp_temperature_deg = data.data_air_bmp_temperature_deg;
  data_air_bmp_altitude_m = data.data_air_bmp_altitude_m;
  data_air_sdp_differentialPressure_Pa = data.data_air_sdp_differentialPressure_Pa;
  data_air_sdp_airspeed_ms = data.data_air_sdp_airspeed_ms;
  data_air_AoA_angle_deg = data.data_air_AoA_angle_deg;
  data_air_AoS_angle_deg = data.data_air_AoS_angle_deg;
  data_ics_angle = data.data_ics_angle;

  // 胴体桁電装用
  fslg_is_alive = data.fslg_is_alive;
  data_fslg_bno_qw = data.data_fslg_bno_qw;
  data_fslg_bno_qx = data.data_fslg_bno_qx;
  data_fslg_bno_qy = data.data_fslg_bno_qy;
  data_fslg_bno_qz = data.data_fslg_bno_qz;
  data_fslg_bno_roll = data.data_fslg_bno_roll;
  data_fslg_bno_pitch = data.data_fslg_bno_pitch;
  data_fslg_bno_yaw = data.data_fslg_bno_yaw;
  data_fslg_lsm_roll = data.data_fslg_lsm_roll;
  data_fslg_lsm_pitch = data.data_fslg_lsm_pitch;
  data_fslg_lsm_yaw = data.data_fslg_lsm_yaw;
  data_fslg_bmp_pressure_hPa = data.data_fslg_bmp_pressure_hPa;
  data_fslg_bmp_temperature_deg = data.data_fslg_bmp_temperature_deg;
  data_fslg_bmp_altitude_m = data.data_fslg_bmp_altitude_m;

  data_fslg_bno_accx_mss = data.data_fslg_bno_accx_mss;
  data_fslg_bno_accy_mss = data.data_fslg_bno_accy_mss;
  data_fslg_bno_accz_mss = data.data_fslg_bno_accz_mss;
  data_fslg_lsm_accx_mss = data.data_fslg_lsm_accx_mss;
  data_fslg_lsm_accy_mss = data.data_fslg_lsm_accy_mss;
  data_fslg_lsm_accz_mss = data.data_fslg_lsm_accz_mss;
  data_fslg_bno_cal_system = data.data_fslg_bno_cal_system;
  data_fslg_bno_cal_gyro = data.data_fslg_bno_cal_gyro;
  data_fslg_bno_cal_accel = data.data_fslg_bno_cal_accel;
  data_fslg_bno_cal_mag = data.data_fslg_bno_cal_mag;

  // Under電装部
  under_is_alive = data.under_is_alive;
  data_under_bmp_pressure_hPa = data.data_under_bmp_pressure_hPa;
  data_under_bmp_temperature_deg = data.data_under_bmp_temperature_deg;
  data_under_bmp_altitude_m = data.data_under_bmp_altitude_m;
  data_under_urm_altitude_m = data.data_under_urm_altitude_m;
  data_under_tsd20_altitude_m = data.data_under_tsd20_altitude_m;
}

// ----------------------------------------------------
// 受信したログデータを文字列に変換
// ----------------------------------------------------
void extractLogData(int readnum) {
  if (readnum == BICO_DATA_NUM) {
    last_Bico_time_ms = millis();

    // 構造体に変換
    LogData data = convertArrayToLogData(Bico_UART.UART_data);
    
    // グローバル変数に代入
    applyLogDataToGlobals(data);
  }
  
  // 生存判定 (1秒 = 1000ms タイムアウト)
  if (millis() - last_Bico_time_ms > 1000) {
    Bico_is_alive = false;
  } else {
    Bico_is_alive = true;
  }
}
