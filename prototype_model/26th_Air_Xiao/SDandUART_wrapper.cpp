#include "SDandUART_wrapper.h"
#include "SD_Air_xiao.h"
#include "UARTHelper_air_xiao.h"
#include "SerialWebHelper.h"
#include <TORICA_UART.h>

QueueHandle_t uartQueue = NULL;  // UART受信データをCore0に送るためのキュー
extern TORICA_UART Bico_UART;    // UARTHelper_air_xiao.cppで定義されているBico_UARTを外部参照


// この関数はsetup()内で呼び出す
void setupSDandUART() {
  // キューを作成．100Hzで50個，つまり500ms分の遅延を吸収するバッファを確保．
  uartQueue = xQueueCreate(50, sizeof(UARTData));

  initSD();       // SD初期化
  flashHeader();  // csvヘッダー書き込み
  initUART();     // UART初期化
}

// void copyLogDataToSDQueue(LogData *txData) {
//     if (txData == NULL) {
//         return; // 引数がNULLの場合
//     }

//     txData->takeoff = takeoff;
//     txData->time_ms = time_ms;
//     txData->filtered_bmp_altitude_m = filtered_bmp_altitude_m;
//     txData->filtered_urm_altitude_m = filtered_urm_altitude_m;
//     txData->urm_is_reliable = urm_is_reliable;
//     txData->filtered_airspeed_ms = filtered_airspeed_ms;

//     // エアデータ
//     txData->data_air_bmp_pressure_hPa = data_air_bmp_pressure_hPa;
//     txData->data_air_bmp_temperature_deg = data_air_bmp_temperature_deg;
//     txData->data_air_bmp_altitude_m = data_air_bmp_altitude_m;
//     txData->data_air_gps_hour = data_air_gps_hour;
//     txData->data_air_gps_minute = data_air_gps_minute;
//     txData->data_air_gps_second = data_air_gps_second;
//     txData->data_air_gps_centisecond = data_air_gps_centisecond;
//     txData->data_air_gps_latitude_deg = data_air_gps_latitude_deg;
//     txData->data_air_gps_longitude_deg = data_air_gps_longitude_deg;
//     txData->data_air_gps_altitude_m = data_air_gps_altitude_m;
//     txData->data_air_gps_groundspeed_ms = data_air_gps_groundspeed_ms;
//     txData->data_air_gps_heading_deg = data_air_gps_heading_deg;
//     txData->data_air_sdp_differentialPressure_Pa = data_air_sdp_differentialPressure_Pa;
//     txData->data_air_sdp_airspeed_ms = data_air_sdp_airspeed_ms;
//     txData->data_air_AoA_angle_deg = data_air_AoA_angle_deg;
//     txData->data_air_AoS_angle_deg = data_air_AoS_angle_deg;
//     txData->data_ics_angle = data_ics_angle;
//     // 胴体桁基板
//     txData->fslg_is_alive = fslg_is_alive;
//     txData->data_fslg_bno_accx_mss = data_fslg_bno_accx_mss;
//     txData->data_fslg_bno_accy_mss = data_fslg_bno_accy_mss;
//     txData->data_fslg_bno_accz_mss = data_fslg_bno_accz_mss;
//     txData->data_fslg_bno_qw = data_fslg_bno_qw;
//     txData->data_fslg_bno_qx = data_fslg_bno_qx;
//     txData->data_fslg_bno_qy = data_fslg_bno_qy;
//     txData->data_fslg_bno_qz = data_fslg_bno_qz;
//     txData->data_fslg_bno_roll = data_fslg_bno_roll;
//     txData->data_fslg_bno_pitch = data_fslg_bno_pitch;
//     txData->data_fslg_bno_yaw = data_fslg_bno_yaw;
//     txData->data_fslg_bno_cal_system = data_fslg_bno_cal_system;
//     txData->data_fslg_bno_cal_gyro = data_fslg_bno_cal_gyro;
//     txData->data_fslg_bno_cal_accel = data_fslg_bno_cal_accel;
//     txData->data_fslg_bno_cal_mag = data_fslg_bno_cal_mag;
//     txData->data_fslg_bmp_pressure_hPa = data_fslg_bmp_pressure_hPa;
//     txData->data_fslg_bmp_temperature_deg = data_fslg_bmp_temperature_deg;
//     txData->data_fslg_bmp_altitude_m = data_fslg_bmp_altitude_m;
//     txData->data_fslg_lsm_accx_mss = data_fslg_lsm_accx_mss;
//     txData->data_fslg_lsm_accy_mss = data_fslg_lsm_accy_mss;
//     txData->data_fslg_lsm_accz_mss = data_fslg_lsm_accz_mss;
//     txData->data_fslg_lsm_roll = data_fslg_lsm_roll;
//     txData->data_fslg_lsm_pitch = data_fslg_lsm_pitch;
//     txData->data_fslg_lsm_yaw = data_fslg_lsm_yaw;
//     // Under基板
//     txData->under_is_alive = under_is_alive;
//     txData->data_under_bmp_pressure_hPa = data_under_bmp_pressure_hPa;
//     txData->data_under_bmp_temperature_deg = data_under_bmp_temperature_deg;
//     txData->data_under_bmp_altitude_m = data_under_bmp_altitude_m;
//     txData->data_under_urm_altitude_m = data_under_urm_altitude_m;
//     txData->data_under_tsd20_altitude_m = data_under_tsd20_altitude_m;


//     // キューにデータを送信
//     // txData自体がLogData構造体のアドレスなのでtxDataに&をつける必要はない．
//     xQueueSend(sdQueue, txData, portMAX_DELAY); // データが送信されるまで待機．第3引数を0にすることで，万が一満杯でも待たずに次の処理に進むことができる．
// }


// Core1でUARTを受信し，Core0に送るタスク
void sendUARTbuff(void *args) {
  UARTData txData;
  static char combined_data[512];
  static int send_counter = 0;
  // 各バッファの初期化


  if (Bico_UART.listenUART()) {
    strncpy(txData.text, Bico_UART.buff, sizeof(txData.text));
    xQueueSend(uartQueue, &txData, 0);
  }
}


// ==========================================
// Core0：SerialWeb用Queue受信 -> パース -> Web送信
// ==========================================
void processCore0_ParseAndWeb() {
  UARTData rxData;

  // 1秒に1回Core1から1行分のデータ（バッファ）が届く
  if (xQueueReceive(uartQueue, &rxData, portMAX_DELAY)) {

    // バッファを各データに分解
    int parsed_num = Bico_UART.parseBuffer(rxData.text);

    // 53個揃っているかチェック
    if (parsed_num == 53) {
      extractLogData(parsed_num);  // 53個入っているかの確認はextractLogData()内でも行われているので，意味はないね
    }
    sendSerialWeb();
  }
}


void processCore1_UARTtoSD() {
  static int one_second_counter = 0;
  UARTData txData;

  if (Bico_UART.listenUART()) {

    // TORICA_UART.listenUART() は末尾の'\n'を'\0'に書き換えてしまう仕様なので、
    // SDやQueueに送るために，改めて末尾に'\n'を付け直してtxDataに入れる
    snprintf(txData.text, sizeof(txData.text), "%s\n", Bico_UART.buff);

    // 完成した1行をSDカードバッファに書き込む (25Hzなら40ms間隔)
    writeBufToSD(txData.text);

    // 1秒 (25Hzなら25回) カウントする
    one_second_counter++;
    if (one_second_counter >= 25) {
      writeSD();
      // 1秒に1回Core0へ送信
      xQueueSend(uartQueue, &txData, 0);
      one_second_counter = 0;  // カウンターをリセット
    }
  }
}