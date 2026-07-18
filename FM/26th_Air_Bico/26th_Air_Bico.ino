/*
Core0: BMP, SDP, AS5600×2
Core1: UART送受信，高度・対気速度計算
*/

// #define DEBUG_MODE  // デバッグモード

#include <Arduino.h>
#include "parameters.h"
#include "Bico_config.h"
#include "pico/multicore.h"
#include "pico/mutex.h"

struct SharedSensorData {
    float air_pressure_hPa;
    float air_temperature_deg;
    float under_pressure_hPa;
    float under_temperature_deg;
    float fslg_pressure_hPa;
    float fslg_temperature_deg;
    float under_urm_altitude_m;
    float sdp_differentialPressure_Pa;
};

static SharedSensorData shared_sensor_data;
static mutex_t sensor_mutex;

// 各ファイル読み込み
#include "calculate_altitude.h"
#include "calculate_airspeed.h"
#include "AS5600.h"
#include "BMP3xx.h"
#include "SDP31.h"
#include "UARTHelper_Bico.h"
#include "GPSHelper.h"
#include "sensor_reader.h"


// 100Hzタイマー用
#include "pico/stdlib.h"
struct repeating_timer core0_timer;
volatile bool core0_timer_triggered = false;  //100Hz用フラグ
struct repeating_timer core1_timer;
volatile bool core1_timer_triggered = false;  //100Hz用フラグ

bool core0_timer_callback(struct repeating_timer *t) {
  core0_timer_triggered = true;
  return true;
}

bool core1_timer_callback(struct repeating_timer *t) {
  core1_timer_triggered = true;
  return true;
}

// Watchdog用
#include "hardware/watchdog.h"
volatile bool core1_alive;  // core1の生存確認用フラグ

void setup() {
  // Mutexの初期化
  mutex_init(&sensor_mutex);

  //LED初期化
  pinMode(LED_ICS, OUTPUT);
  pinMode(LED_Under, OUTPUT);
  pinMode(LED_Air_pico, OUTPUT);
  pinMode(LED_Air_xiao, OUTPUT);
  pinMode(LED_GPS, OUTPUT);
  pinMode(LED_SD, OUTPUT);

  Serial.begin(115200, SERIAL_8E1);  //DEBUG用USB-UART

  //ESP用・Under用UART初期化
  initUART();
  initUART_DMA(); // DMA送信の初期化

  // GPS初期化
  initGPS();

  // SD内csv用ヘッダー送信
  transmitHeader();

  // Bico I2C0初期化動作
  Wire.setSDA(bico_I2C0_SDA);
  Wire.setSCL(bico_I2C0_SCL);
  Wire1.setSDA(bico_I2C1_SDA);
  Wire1.setSCL(bico_I2C1_SCL);
  Wire.begin();
  Wire1.begin();
  Wire.setClock(400000);
  Wire1.setClock(400000);

// USB接続時のために起動待機（7秒）
// #ifdef DEBUG_MODE  //DEBUG_MODEが有効ならば
//   for (int i = 1; i <= 7; i++) {
//     digitalWrite(LED_ICS, HIGH);
//     digitalWrite(LED_Under, HIGH);
//     digitalWrite(LED_Air_pico, HIGH);
//     digitalWrite(LED_Air_xiao, HIGH);
//     digitalWrite(LED_GPS, HIGH);
//     digitalWrite(LED_SD, HIGH);
//     delay(500);
//     digitalWrite(LED_ICS, LOW);
//     digitalWrite(LED_Under, LOW);
//     digitalWrite(LED_Air_pico, LOW);
//     digitalWrite(LED_Air_xiao, LOW);
//     digitalWrite(LED_GPS, LOW);
//     digitalWrite(LED_SD, LOW);
//     delay(500);
//   }
//   Serial.println("DEBUG MODE Enabled");
// #endif  //DEBUG_MODEが有効ならば

  // 各センサーの初期化
  if (SDP31_init(&Wire, 0x23)) {
    Serial.println("SDP init done");
  } else {
    Serial.println("SDP init failed");
  }

  if (AS5600_init()) {
    Serial.println("AS5600x2 setup done");
  } else {
    Serial.println("AS5600x2 setup failed");
  }

  if (BMP3XX_init(&Wire1, 0x76)) {
    Serial.println("BMP setup done");
  } else {
    Serial.println("BMP setup failed");
  }

  watchdog_enable(2000, 1);  // watchdogを有効化．
  /* 2000ms(=2s)経っても反応がない場合，システムが暴走したとみなして強制再起動 */

  // ハードウェアタイマー起動
  add_repeating_timer_ms(-10, core0_timer_callback, NULL, &core0_timer);

  Serial.println("All setup is done");
}


// CPU1のセットアップ
void setup1() {
  // ハードウェアタイマーの設定はコアごとに
  add_repeating_timer_ms(-10, core1_timer_callback, NULL, &core1_timer);
}


void loop() {
  if (core0_timer_triggered == true) {

    core0_timer_triggered = false;  // タイマーのフラグを戻す

    digitalWrite(LED_ICS, HIGH);
    digitalWrite(LED_Under, HIGH);
    digitalWrite(LED_Air_pico, HIGH);
    digitalWrite(LED_Air_xiao, HIGH);
    digitalWrite(LED_GPS, HIGH);
    digitalWrite(LED_SD, HIGH);

    time_ms = millis();  // センサー読み取り時刻を記録

    // 各センサーの値を読み取り、グローバル変数に代入
    update_air_bmp();
    update_air_AS5600();
    update_air_SDP();

    // GPSは10Hzつまり100msに一回読む
    static int gps_counter = 0;
    if (gps_counter >= 10) {
      update_air_gps();
      gps_counter = 0;
    }
    gps_counter++;

    // Core1へデータを安全に渡すため、Mutexロックを取得して共有領域へコピー
    mutex_enter_blocking(&sensor_mutex);
    shared_sensor_data.air_pressure_hPa = data_air_bmp_pressure_hPa;
    shared_sensor_data.air_temperature_deg = data_air_bmp_temperature_deg;
    shared_sensor_data.under_pressure_hPa = data_under_bmp_pressure_hPa;
    shared_sensor_data.under_temperature_deg = data_under_bmp_temperature_deg;
    shared_sensor_data.fslg_pressure_hPa = data_fslg_bmp_pressure_hPa;
    shared_sensor_data.fslg_temperature_deg = data_fslg_bmp_temperature_deg;
    shared_sensor_data.under_urm_altitude_m = data_under_urm_altitude_m;
    shared_sensor_data.sdp_differentialPressure_Pa = data_air_sdp_differentialPressure_Pa;
    mutex_exit(&sensor_mutex);

    // Core1へ書き込み完了シグナル（値1）を送る
    multicore_fifo_push_blocking(1);
    

    // static int debug_counter = 0;
    // if (debug_counter > 100){
    // Serial.print("time_ms:  ");
    // Serial.println(time_ms);
    // Serial.print("bmp:  ");
    // Serial.println(data_air_bmp_pressure_hPa);
    // Serial.print("AoA:  ");
    // Serial.println(data_air_AoA_angle_deg);
    // Serial.print("AoS:  ");
    // Serial.println(data_air_AoS_angle_deg);
    // Serial.print("SDP:  ");
    // Serial.println(data_air_sdp_differentialPressure_Pa);
    // Serial.print("airspeed:  ");
    // Serial.println(data_air_sdp_airspeed_ms);
    // Serial.print("GPS:  ");
    // Serial.println(data_air_gps_second);
    // debug_counter = 0;
    // }
    // debug_counter++;


    digitalWrite(LED_ICS, LOW);
    digitalWrite(LED_Under, LOW);
    digitalWrite(LED_Air_pico, LOW);
    digitalWrite(LED_Air_xiao, LOW);
    digitalWrite(LED_GPS, LOW);
    digitalWrite(LED_SD, LOW);


    // Core1生存確認
    if (core1_alive == true) {
      watchdog_update();  // Watchdogに合図を送る

      core1_alive = false;  // core1生存フラグを戻す
    }
  }
}

void loop1() {
  if (core1_timer_triggered == true) {
    core1_timer_triggered = false;  // タイマーのフラグを戻す

    // 機体下・胴体桁・ICS・ESP信号読み取り
    receiveUnderLog();
    receiveFslgLog();
    receiveIcsAngle();
    handleEspSignal();

    // コア間で同じ変数に同時アクセスがあった場合，数値が破損する場合がある．これを避けるために，コア間でデータのやり取りをする場合，いったんFIFOにデータを格納して，受け渡す．
    // Core0からのデータ準備完了シグナルがあるかチェックし，ある場合は安全にデータをコピーして計算を行う
    float local_air_press, local_air_temp;
    float local_under_press, local_under_temp;
    float local_fslg_press, local_fslg_temp;
    float local_under_urm;
    float local_sdp_diff;

    if (multicore_fifo_rvalid()) {
      multicore_fifo_pop_blocking(); // シグナルをポップ

      mutex_enter_blocking(&sensor_mutex);
      local_air_press = shared_sensor_data.air_pressure_hPa;
      local_air_temp  = shared_sensor_data.air_temperature_deg;
      local_under_press = shared_sensor_data.under_pressure_hPa;
      local_under_temp  = shared_sensor_data.under_temperature_deg;
      local_fslg_press = shared_sensor_data.fslg_pressure_hPa;
      local_fslg_temp  = shared_sensor_data.fslg_temperature_deg;
      local_under_urm   = shared_sensor_data.under_urm_altitude_m;
      local_sdp_diff   = shared_sensor_data.sdp_differentialPressure_Pa;
      mutex_exit(&sensor_mutex);
    } else {
      // シグナルがない場合は既存の値をそのまま使用
      local_air_press = data_air_bmp_pressure_hPa;
      local_air_temp  = data_air_bmp_temperature_deg;
      local_under_press = data_under_bmp_pressure_hPa;
      local_under_temp  = data_under_bmp_temperature_deg;
      local_fslg_press = data_fslg_bmp_pressure_hPa;
      local_fslg_temp  = data_fslg_bmp_temperature_deg;
      local_under_urm   = data_under_urm_altitude_m;
      local_sdp_diff   = data_air_sdp_differentialPressure_Pa;
    }

    // 高度計算（安全にコピーされた引数を渡す）
    calculate_altitude(local_air_press, local_air_temp, local_under_press, local_under_temp, local_fslg_press, local_fslg_temp, local_under_urm);

    // 対気速度計算（安全にコピーされた引数を渡す）
    calculate_airspeed(local_sdp_diff, local_air_temp, local_air_press);

    // LiDARのデータをもとにtakeoff判定

    // takeoff == falseのときのみ判定．trueからfalseへはスマホからでしか許可しない．
    if (takeoff == false){
      is_takeoff();
    }

    // UART送信用カウント変数
    static int transmit_count = 0;

    // UART送信
    transmitLog(transmit_count);
    transmit_count++;
    // 一通り送信(=transmit_countが4以上)したらカウントリセット
    if (transmit_count > 3) {
      transmit_count = 0;
    }

    // 胴体桁送信用カウント変数
    static int transmit_count_fslg = 0;
    transmitLog_for_fslg(transmit_count_fslg);
    transmit_count_fslg++;
    if (transmit_count_fslg > 2){
      transmit_count_fslg = 0;
    }

    core1_alive = true;  // core1生存フラグを立てる
  }
}
