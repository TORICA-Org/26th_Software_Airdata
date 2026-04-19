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
  Serial1.setRxBufferSize(1024); // バッファ(受信したデータの一時保管場所)サイズ指定(1024byte)

  // パラメータ設定とともに通信を開始
  // ICS通信の仕様に合わせ，`SERIAL_8E1`としている．
  // `8`:データビットの長さ
  // `E`:偶数パリティ(`N`:パリティなし，`O`:奇数パリティ)
  // `1`:ストップビット(データフレームの終わりを示すビット)の長さ
  Serial1.begin(460800, SERIAL_8E1, BICO_UART_RX, BICO_UART_TX);
}


void receiveLog() {
  static unsigned long int last_Bico_time_ms = 0;
  int readnum = Bico_UART.readUART();
  const int Bico_data_num = 45;  // 正常な場合のデータ受信数
  bool Bico_is_alive = false; // Bicoが生きているかどうか

  if (readnum == Bico_data_num) {
    last_Bico_time_ms = millis();
    // 受信データを格納

    // 1回目の受信 11個
    time_ms = Bico_UART.UART_data[0];
    takeoff = Bico_UART.UART_data[1];
    urm_is_reliable = static_cast<bool>(Bico_UART.UART_data[2]); // TORICA_UARTはUART_data[]をfloat型で返すためboolに変換
    data_air_gps_hour = Bico_UART.UART_data[3];
    data_air_gps_minute = Bico_UART.UART_data[4];
    data_air_gps_second = Bico_UART.UART_data[5];
    data_air_gps_centisecond = Bico_UART.UART_data[6];
    data_air_gps_latitude_deg = Bico_UART.UART_data[7];
    data_air_gps_longitude_deg = Bico_UART.UART_data[8];
    data_air_gps_altitude_m = Bico_UART.UART_data[9];
    data_air_gps_groundspeed_ms = Bico_UART.UART_data[10];

    // 2回目の受信 10個
    filtered_bmp_altitude_m = Bico_UART.UART_data[11];
    filtered_urm_altitude_m = Bico_UART.UART_data[12];
    data_air_bmp_pressure_hPa = Bico_UART.UART_data[13];
    data_air_bmp_temperature_deg = Bico_UART.UART_data[14];
    data_air_bmp_altitude_m = Bico_UART.UART_data[15];
    data_air_sdp_differentialPressure_Pa = Bico_UART.UART_data[16];
    data_air_sdp_airspeed_ms = Bico_UART.UART_data[17];
    data_air_AoA_angle_deg = Bico_UART.UART_data[18];
    data_air_AoS_angle_deg = Bico_UART.UART_data[19];
    data_ics_angle = Bico_UART.UART_data[20];

    // 3回目の受信 11個
    psd_is_alive = static_cast<bool>(Bico_UART.UART_data[21]);
    data_psd_bno_qw = Bico_UART.UART_data[22];
    data_psd_bno_qx = Bico_UART.UART_data[23];
    data_psd_bno_qy = Bico_UART.UART_data[24];
    data_psd_bno_qz = Bico_UART.UART_data[25];
    data_psd_bno_roll = Bico_UART.UART_data[26];
    data_psd_bno_pitch = Bico_UART.UART_data[27];
    data_psd_bno_yaw = Bico_UART.UART_data[28];
    data_psd_bmp_pressure_hPa = Bico_UART.UART_data[29];
    data_psd_bmp_temperature_deg = Bico_UART.UART_data[30];
    data_psd_bmp_altitude_m = Bico_UART.UART_data[31];

    // 4回目の受信 13個
    data_psd_bno_accx_mss = Bico_UART.UART_data[32];
    data_psd_bno_accy_mss = Bico_UART.UART_data[33];
    data_psd_bno_accz_mss = Bico_UART.UART_data[34];
    data_psd_bno_cal_system = Bico_UART.UART_data[35];
    data_psd_bno_cal_gyro = Bico_UART.UART_data[36];
    data_psd_bno_cal_accel = Bico_UART.UART_data[37];
    data_psd_bno_cal_mag = Bico_UART.UART_data[38];
    under_is_alive = static_cast<bool>(Bico_UART.UART_data[39]);
    data_under_bmp_pressure_hPa = Bico_UART.UART_data[40];
    data_under_bmp_temperature_deg = Bico_UART.UART_data[41];
    data_under_bmp_altitude_m = Bico_UART.UART_data[42];
    data_under_urm_altitude_m = Bico_UART.UART_data[43];
    data_under_tsd20_altitude_m = Bico_UART.UART_data[44];

  }

  //最終受信時間から1秒以上経過している場合はBicoが死んでいるとみなす
  if (millis() - last_Bico_time_ms > 1000) {
    Bico_is_alive = false;
  } else {
    Bico_is_alive = true;
  }
}
