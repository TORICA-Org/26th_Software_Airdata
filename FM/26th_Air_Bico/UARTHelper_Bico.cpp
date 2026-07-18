#include <Arduino.h>
#include "UARTHelper_Bico.h"
#include "Bico_config.h"
#include "hardware/dma.h"
#include "hardware/uart.h"

static int uart_dma_chan = -1;
static dma_channel_config dma_cfg;
static char dma_trans_buff[1024];

#define Serial_ICS Serial1    //ICSのUART // `Serial_ICS`を`Serial1`としてマクロを登録
#define Serial_ESP Serial2    // エアデータとESP32との通信

//PIO UARTの宣言
// SerialPIO Serial_ESP(Serial_Air_xiao_TX, Serial_Air_xiao_RX, 1024);  // エアデータESP32との通信
SerialPIO Serial_fslg(Serial_fslg_TX, Serial_fslg_RX, 1024); // 胴体桁基板(fslg)との通信
SerialPIO Serial_Under(Serial_Under_TX, Serial_Under_RX, 1024);
// SerialPIO.h内プロトタイプ宣言：`SerialPIO(pin_size_t tx, pin_size_t rx, size_t fifoSize = 32);`
// 本プログラムにおいては`fifosize`について考慮の余地あり


// TORICA_UARTインスタンス化
#include <TORICA_UART.h>
TORICA_UART ESP_UART(&Serial_ESP);
TORICA_UART Under_UART(&Serial_Under);
TORICA_UART Fslg_UART(&Serial_fslg);


// TORICA_ICS
#include <TORICA_ICS.h>
TORICA_ICS ICS(&Serial_ICS);


void initUART() {

  //UARTピン設定
  Serial_ICS.setTX(Serial_ICS_TX);
  Serial_ICS.setRX(Serial_ICS_RX);
  Serial_ESP.setTX(Serial_Air_xiao_TX);
  Serial_ESP.setRX(Serial_Air_xiao_RX);

  // UART初期化（<-まだ通信の開始処理はされていない）
  Serial_ICS.setFIFOSize(2048);    // バッファ(受信したデータの一時保管場所)サイズ指定(2048byte)
  // Serial_Under.setFIFOSize(2048);  // バッファ(受信したデータの一時保管場所)サイズ指定(2048byte)

  // パラメータ設定とともに通信を開始
  // ICS通信の仕様に合わせ，`SERIAL_8E1`としている．
  // `8`:データビットの長さ
  // `E`:偶数パリティ(`N`:パリティなし，`O`:奇数パリティ)
  // `1`:ストップビット(データフレームの終わりを示すビット)の長さ
  // デフォルトでは`SERIAL_8N1`となっている．
  Serial_ICS.begin(115200, SERIAL_8E1);
  Serial_ESP.begin(460800, SERIAL_8E1);
  Serial_Under.begin(460800, SERIAL_8E1);
  Serial_fslg.begin(460800, SERIAL_8E1);

  // Serial.begin(115200);  // デバッグ用にパリティはいらないかな...ってか使えない気がする
  Serial.print("loading...\n\n");
}


void initUART_DMA() {
#ifdef USE_DMA
  uart_dma_chan = dma_claim_unused_channel(true);
  dma_cfg = dma_channel_get_default_config(uart_dma_chan);

  channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_8); // 1バイト転送
  channel_config_set_read_increment(&dma_cfg, true);          // 読み出し側は進める
  channel_config_set_write_increment(&dma_cfg, false);        // 書き込み側は固定
  
  // DREQ_UART1_TX (Serial_ESP / Serial2 は RP2040 の uart1 実体)
  channel_config_set_dreq(&dma_cfg, DREQ_UART1_TX);
#endif
}


void transmitHeader() {
  char trans_buff[1024];
  // この関数は`setup()`内なのでブロッキング関数（処理の流れが止まる関数）であっても構わない
  const char *str[3];

  for (int i = 0; i < 4 /* case0~3まで実行 */; i++) {

    switch (i) {
      case 0: // 13個
      { 
        str[0] = "time_ms,takeoff,urm_is_reliable,data_air_gps_hour,"; // 4個
        str[1] = "data_air_gps_minute,data_air_gps_second,data_air_gps_centisecond, data_air_gps_latitude_deg,"; // 4個
        str[2] = "data_air_gps_longitude_deg,data_air_gps_altitude_m, data_air_gps_groundspeed_ms,data_air_gps_heading_deg,data_air_gps_satellites,"; // 5個  
        break;
        }
      case 1:
      { // 11個
        str[0] = "filtered_bmp_altitude_m,filtered_urm_altitude_m,filtered_airspeed_ms,data_air_bmp_pressure_hPa,"; // 4個
        str[1] = "data_air_bmp_temperature_deg,data_air_bmp_altitude_m,data_air_sdp_differentialPressure_Pa,data_air_sdp_airspeed_ms,"; // 4個
        str[2] = "data_air_AoA_angle_deg,data_air_AoS_angle_deg,data_ics_angle,"; // 3個
        break;
        }
      case 2: // 14個
      {
        str[1] = "fslg_is_alive,data_fslg_bno_qw,data_fslg_bno_qx,data_fslg_bno_qy,data_fslg_bno_qz,"; // 5個
        str[2] = "data_fslg_bno_roll,data_fslg_bno_pitch,data_fslg_bno_yaw,data_fslg_lsm_roll,data_fslg_lsm_pitch,"; // 5個
        str[2] = "data_fslg_lsm_yaw,data_fslg_bmp_pressure_hPa,data_fslg_bmp_temperature_deg,data_fslg_bmp_altitude_m"; // 4個
        break;
        }
      case 3: // 16個
      {
        str[0] = "data_fslg_bno_accx_mss,data_fslg_bno_accy_mss,data_fslg_bno_accz_mss,data_fslg_lsm_accx_mss,data_fslg_lsm_accy_mss, data_fslg_lsm_accz_mss,"; // 6個
        str[1] = "data_fslg_bno_cal_system,data_fslg_bno_cal_gyro,data_fslg_bno_cal_accel,data_fslg_bno_cal_mag,under_is_alive,"; // 5個
        str[2] = "data_under_bmp_pressure_hPa,data_under_bmp_temperature_deg,data_under_bmp_altitude_m,data_under_urm_altitude_m, data_under_tsd20_altitude_m\n"; // 5個
        break;
      }
      default:
      {
        Serial.println("The parameter value is out of range.");
        break;
      }
    }
    sprintf(trans_buff, "%s%s%s", str[0], str[1], str[2]);

    //バッファをクリアしてから新しいデータを書き込み
    // Serial_ESP.flush();
    Serial_Under.flush();
    // Serial_ESP.print(trans_buff);
    Serial_Under.print(trans_buff);

    delayMicroseconds(10);  // 遅延あったほうがいいと思う
  }
}

/*------------------------------------------------------------

各trans_modeにおいて送信するデータ量を均等にするためにこの順番にしている． 
ASCIIコードに変換すると，
特大：GPS緯度経度 -> %.7fなので12~13byte
大：気圧・GPS高度・time_ms -> %.2f, %uなので8byte
中：速度・温度・IMUなど -> 6~7byte
小：時刻・ICS角度(int) -> 3~5byte
極小：フラグ・キャリブレーション値 -> 2byte 

-----------------------------------------------------------*/


void transmitLog(int trans_mode) {  // 関数分けるのは面倒なので引数（0~3）でモード変更
  char trans_buff[1024];
  switch (trans_mode) { // 計53個
    case 0: // 計13個
      {
        sprintf(trans_buff, "%lu,%d,%d,%u,%u,%u,%u,%.7f,%.7f,%.2f,%.2f,%.1f,%u,", 
        time_ms, takeoff, urm_is_reliable, data_air_gps_hour, // 4個
        data_air_gps_minute, data_air_gps_second, data_air_gps_centisecond, data_air_gps_latitude_deg, // 4個
        data_air_gps_longitude_deg, data_air_gps_altitude_m, data_air_gps_groundspeed_ms,data_air_gps_heading_deg,data_air_gps_satellites // 5個
        );
        break;
      }
    case 1:
      { // 計11個
        sprintf(trans_buff, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,", 
        filtered_bmp_altitude_m, filtered_urm_altitude_m, filtered_airspeed_ms, // 3個
        data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg, data_air_bmp_altitude_m, // 3個
        data_air_sdp_differentialPressure_Pa, data_air_sdp_airspeed_ms, // 2個
        data_air_AoA_angle_deg, data_air_AoS_angle_deg, data_ics_angle); // 3個
        break;
      }
    case 2: // 計14個
      {
        sprintf(trans_buff, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,", 
        fslg_is_alive, // 1個
        data_fslg_bno_qw, data_fslg_bno_qx, data_fslg_bno_qy, data_fslg_bno_qz, // 4個
        data_fslg_bno_roll, data_fslg_bno_pitch, data_fslg_bno_yaw, // 3個
        data_fslg_lsm_roll, data_fslg_lsm_pitch, data_fslg_lsm_yaw, // 3個
        data_fslg_bmp_pressure_hPa, data_fslg_bmp_temperature_deg, data_fslg_bmp_altitude_m); // 3個
        break;
      }

    case 3: // 計16個
      {
        sprintf(trans_buff, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%u,%u,%u,%u,%d,%.2f,%.2f,%.2f,%.2f,%.2f\n", 
        data_fslg_bno_accx_mss, data_fslg_bno_accy_mss, data_fslg_bno_accz_mss, // 3個
        data_fslg_lsm_accx_mss, data_fslg_lsm_accy_mss, data_fslg_lsm_accz_mss, // 3個
        data_fslg_bno_cal_system, data_fslg_bno_cal_gyro, data_fslg_bno_cal_accel, data_fslg_bno_cal_mag, // 4個
        under_is_alive, // 1個
        data_under_bmp_pressure_hPa, data_under_bmp_temperature_deg, data_under_bmp_altitude_m, // 3個
        data_under_urm_altitude_m, data_under_tsd20_altitude_m); // 2個
        break;
      }
    default:
      {
        Serial.println("The parameter value is out of range.");
        break;
      }
  }

#ifdef USE_DMA
  // 前回のDMA転送が終わっていない場合はスキップ（ノンブロッキング処理にするため）
  if (uart_dma_chan != -1) {
    if (dma_channel_is_busy(uart_dma_chan)) {
      return;
    }
  }

  // DMA送信バッファにコピー
  size_t len = strlen(trans_buff);
  if (len >= sizeof(dma_trans_buff)) {
    len = sizeof(dma_trans_buff) - 1;
  }
  memcpy(dma_trans_buff, trans_buff, len);
  dma_trans_buff[len] = '\0';

  // DMAチャネルを設定して即時送信開始（非同期）
  if (uart_dma_chan != -1) {
    dma_channel_configure(
        uart_dma_chan,
        &dma_cfg,
        &uart1_hw->dr,
        dma_trans_buff,
        len,
        true
    );
  }
#else
  // 通常の同期送信（DMA未使用時）
  Serial_ESP.flush();
  Serial_ESP.print(trans_buff);
#endif

  // ソフトウェアシリアル（Under側）は通常どおり同期送信
  Serial_Under.flush();
  Serial_Under.print(trans_buff);
  // Serial_fslg.print(trans_buff);
}


// 胴体桁向け送信関数
void transmitLog_for_fslg(int trans_mode) {  // 関数分けるのは面倒なので引数（0~3）でモード変更
  char trans_buff[512];
  switch (trans_mode) { // 計29個
    case 0: // 計13個
      {
        sprintf(trans_buff, "%lu,%d,%d,%u,%u,%u,%u,%.7f,%.7f,%.2f,%.2f,%.1f,%u,", 
        time_ms, takeoff, urm_is_reliable, data_air_gps_hour, // 4個
        data_air_gps_minute, data_air_gps_second, data_air_gps_centisecond, data_air_gps_latitude_deg, // 4個
        data_air_gps_longitude_deg, data_air_gps_altitude_m, data_air_gps_groundspeed_ms,data_air_gps_heading_deg, data_air_gps_satellites // 5個
        );
        break;
      }
    case 1:
      { // 計11個
        sprintf(trans_buff, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,", 
        filtered_bmp_altitude_m, filtered_urm_altitude_m, filtered_airspeed_ms, // 3個
        data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg, data_air_bmp_altitude_m, // 3個
        data_air_sdp_differentialPressure_Pa, data_air_sdp_airspeed_ms, // 2個
        data_air_AoA_angle_deg, data_air_AoS_angle_deg, data_ics_angle); // 3個
        break;
      }

    case 2: // 計6個
      {
        sprintf(trans_buff, "%d,%.2f,%.2f,%.2f,%.2f,%.2f\n",
        under_is_alive, // 1個
        data_under_bmp_pressure_hPa, data_under_bmp_temperature_deg, data_under_bmp_altitude_m, // 3個
        data_under_urm_altitude_m, data_under_tsd20_altitude_m // 2個
        );
        break;
      }
    default:
      {
        Serial.println("The parameter value is out of range.");
        break;
      }
  }

  //バッファをクリアしてから新しいデータを書き込み
  Serial_fslg.flush();
  Serial_fslg.print(trans_buff);
}


void receiveUnderLog() {
  // 機体下受信
  static unsigned long int last_under_time_ms = 0;
  int readnum_under = Under_UART.readUART();
  const int under_data_num = 5;  // 正常な場合のデータ受信数

  if (readnum_under == under_data_num) {
    last_under_time_ms = millis();
    // 受信データを格納
    data_under_bmp_pressure_hPa = Under_UART.UART_data[0];
    data_under_bmp_temperature_deg = Under_UART.UART_data[1];
    data_under_bmp_altitude_m = Under_UART.UART_data[2];
    data_under_urm_altitude_m = Under_UART.UART_data[3];
    data_under_tsd20_altitude_m = Under_UART.UART_data[4];
  }

  // 最終受信時間から1秒以上経過している場合は機体下が死んでいるとみなす
  if (millis() - last_under_time_ms > 1000) {
    under_is_alive = false;
  } else {
    under_is_alive = true;
  }
}


void receiveFslgLog() {
  // 胴体桁受信
  static unsigned long int last_fslg_time_ms = 0;
  int readnum_fslg = Fslg_UART.readUART();
  const int fslg_data_num = 23;  // 正常な場合のデータ受信数

  if (readnum_fslg == fslg_data_num) {
    last_fslg_time_ms = millis();

    // 1回目の受信
    data_fslg_bno_accx_mss = Fslg_UART.UART_data[0];
    data_fslg_bno_accy_mss = Fslg_UART.UART_data[1];
    data_fslg_bno_accz_mss = Fslg_UART.UART_data[2];
    data_fslg_bno_qw = Fslg_UART.UART_data[3];
    data_fslg_bno_qx = Fslg_UART.UART_data[4];
    data_fslg_bno_qy = Fslg_UART.UART_data[5];
    data_fslg_bno_qz = Fslg_UART.UART_data[6];
    data_fslg_bno_roll = Fslg_UART.UART_data[7];
    data_fslg_bno_pitch = Fslg_UART.UART_data[8];
    data_fslg_bno_yaw = Fslg_UART.UART_data[9];
    data_fslg_bno_cal_system = Fslg_UART.UART_data[10];
    data_fslg_bno_cal_gyro = Fslg_UART.UART_data[11];
    data_fslg_bno_cal_accel = Fslg_UART.UART_data[12];
    data_fslg_bno_cal_mag = Fslg_UART.UART_data[13];

    // 2回目の受信
    data_fslg_bmp_pressure_hPa = Fslg_UART.UART_data[14];
    data_fslg_bmp_temperature_deg = Fslg_UART.UART_data[15];
    data_fslg_bmp_altitude_m = Fslg_UART.UART_data[16];
    data_fslg_lsm_accx_mss = Fslg_UART.UART_data[17];
    data_fslg_lsm_accy_mss = Fslg_UART.UART_data[18];
    data_fslg_lsm_accz_mss = Fslg_UART.UART_data[19];
    data_fslg_lsm_roll = Fslg_UART.UART_data[20];
    data_fslg_lsm_pitch = Fslg_UART.UART_data[21];
    data_fslg_lsm_yaw = Fslg_UART.UART_data[22];
  }

  // 最終受信時間から1秒以上経過している場合は胴体桁が死んでいるとみなす
  if (millis() - last_fslg_time_ms > 1000) {
    fslg_is_alive = false;
  } else {
    fslg_is_alive = true;
  }
}


void receiveIcsAngle() {
  // ICS読み取り
  int new_ics_angle = 0;
  new_ics_angle = ICS.read_Angle();
  digitalWrite(LED_ICS, LOW);
  if (new_ics_angle > 0){
    data_ics_angle = new_ics_angle;
    digitalWrite(LED_ICS, HIGH);
  }
}


void handleEspSignal() {
  // エアデータ ESP32 XiaoからRESETやCALIBなどのシグナル受信
  if (ESP_UART.listenUART()){
    // 文字列のどこかに"RESET"が含まれている場合
    if (strstr(ESP_UART.buff, "RESET") != NULL){
      Serial_Under.print("\nRESET\n"); // UnderはUARTを受信するとそのままSD書き込み用バッファに送り込まれる
      Serial_fslg.print("RESET"); // 胴体桁基板は受信すると文字列解析にかけられる．

      // スピーカー強制ON
    } else if (strstr(ESP_UART.buff, "SPK_EN") != NULL) {
      Serial_fslg.print("SPK_EN");

      // スピーカー強制OFF
    } else if (strstr(ESP_UART.buff, "SPK_DIS") != NULL){
      Serial_fslg.print("SPK_DIS");

      // Takeoffフラグを反転させる
    } else if (strstr(ESP_UART.buff, "CHG_TO") != NULL) {
      Serial_fslg.print("CHG_TO");  // 胴体桁基板に送信（スピーカーで使うため）
      takeoff = !takeoff; // takeoffフラグを反転
    }
  }
}