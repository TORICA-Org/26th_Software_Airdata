#include "UARTHelper_air_xiao.h"
#include "parameters.h"
#include "Air_xiao_config.h"
#include <Arduino.h>


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



void extractLogData(int readnum) {
  const int Bico_data_num = 53;  // 正常な場合のデータ受信数

  if (readnum == Bico_data_num) {
    last_Bico_time_ms = millis();

    // 受信データを格納
    // 1回目の受信 12個
    time_ms = Bico_UART.UART_data[0];
    takeoff = Bico_UART.UART_data[1];
    urm_is_reliable = static_cast<bool>(Bico_UART.UART_data[2]);
    data_air_gps_hour = Bico_UART.UART_data[3];
    data_air_gps_minute = Bico_UART.UART_data[4];
    data_air_gps_second = Bico_UART.UART_data[5];
    data_air_gps_centisecond = Bico_UART.UART_data[6];
    data_air_gps_latitude_deg = Bico_UART.UART_data[7];
    data_air_gps_longitude_deg = Bico_UART.UART_data[8];
    data_air_gps_altitude_m = Bico_UART.UART_data[9];
    data_air_gps_groundspeed_ms = Bico_UART.UART_data[10];
    data_air_gps_heading_deg = Bico_UART.UART_data[11];

    // 2回目の受信 11個
    filtered_bmp_altitude_m = Bico_UART.UART_data[12];
    filtered_urm_altitude_m = Bico_UART.UART_data[13];
    filtered_airspeed_ms = Bico_UART.UART_data[14];
    data_air_bmp_pressure_hPa = Bico_UART.UART_data[15];
    data_air_bmp_temperature_deg = Bico_UART.UART_data[16];
    data_air_bmp_altitude_m = Bico_UART.UART_data[17];
    data_air_sdp_differentialPressure_Pa = Bico_UART.UART_data[18];
    data_air_sdp_airspeed_ms = Bico_UART.UART_data[19];
    data_air_AoA_angle_deg = Bico_UART.UART_data[20];
    data_air_AoS_angle_deg = Bico_UART.UART_data[21];
    data_ics_angle = Bico_UART.UART_data[22];

    // 3回目の受信 14個
    fslg_is_alive = static_cast<bool>(Bico_UART.UART_data[23]);
    data_fslg_bno_qw = Bico_UART.UART_data[24];
    data_fslg_bno_qx = Bico_UART.UART_data[25];
    data_fslg_bno_qy = Bico_UART.UART_data[26];
    data_fslg_bno_qz = Bico_UART.UART_data[27];
    data_fslg_bno_roll = Bico_UART.UART_data[28];
    data_fslg_bno_pitch = Bico_UART.UART_data[29];
    data_fslg_bno_yaw = Bico_UART.UART_data[30];
    data_fslg_lsm_roll = Bico_UART.UART_data[31];
    data_fslg_lsm_pitch = Bico_UART.UART_data[32];
    data_fslg_lsm_yaw = Bico_UART.UART_data[33];
    data_fslg_bmp_pressure_hPa = Bico_UART.UART_data[34];
    data_fslg_bmp_temperature_deg = Bico_UART.UART_data[35];
    data_fslg_bmp_altitude_m = Bico_UART.UART_data[36];

    // 4回目の受信 16個
    data_fslg_bno_accx_mss = Bico_UART.UART_data[37];
    data_fslg_bno_accy_mss = Bico_UART.UART_data[38];
    data_fslg_bno_accz_mss = Bico_UART.UART_data[39];
    data_fslg_lsm_accx_mss = Bico_UART.UART_data[40];
    data_fslg_lsm_accy_mss = Bico_UART.UART_data[41];
    data_fslg_lsm_accz_mss = Bico_UART.UART_data[42];
    data_fslg_bno_cal_system = static_cast<uint8_t>(Bico_UART.UART_data[43]);
    data_fslg_bno_cal_gyro = static_cast<uint8_t>(Bico_UART.UART_data[44]);
    data_fslg_bno_cal_accel = static_cast<uint8_t>(Bico_UART.UART_data[45]);
    data_fslg_bno_cal_mag = static_cast<uint8_t>(Bico_UART.UART_data[46]);
    under_is_alive = static_cast<bool>(Bico_UART.UART_data[47]);
    data_under_bmp_pressure_hPa = Bico_UART.UART_data[48];
    data_under_bmp_temperature_deg = Bico_UART.UART_data[49];
    data_under_bmp_altitude_m = Bico_UART.UART_data[50];
    data_under_urm_altitude_m = Bico_UART.UART_data[51];
    data_under_tsd20_altitude_m = Bico_UART.UART_data[52];

    // } else if (readnum > 0) {
    //   // 受信データ数が異なる場合
    //   return;

  } else {
    // 最終受信時間から1秒以上経過している場合はBicoが死んでいるとみなす
    if (millis() - last_Bico_time_ms > 1000) {
      Bico_is_alive = false;
    } else {
      Bico_is_alive = true;
    }
  }
}