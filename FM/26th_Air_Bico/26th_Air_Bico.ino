/*
Core0: BMP, SDP, AS5600×2
Core1: UART送受信，高度・対気速度計算
*/

#define DEBUG_MODE  // デバッグモード

#include <Arduino.h>
#include "parameters.h"
#include "Bico_config.h"

// 各ファイル読み込み
#include "calculate_altitude.h"
#include "calculate_airspeed.h"
#include "AS5600.h"
#include "BMP3xx.h"
#include "SDP31.h"
#include "UARTHelper_Bico.h"
#include "GPSHelper.h"


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
#ifdef DEBUG_MODE  //DEBUG_MODEが有効ならば
  for (int i = 1; i <= 7; i++) {
    digitalWrite(LED_ICS, HIGH);
    digitalWrite(LED_Under, HIGH);
    digitalWrite(LED_Air_pico, HIGH);
    digitalWrite(LED_Air_xiao, HIGH);
    digitalWrite(LED_GPS, HIGH);
    digitalWrite(LED_SD, HIGH);
    delay(500);
    digitalWrite(LED_ICS, LOW);
    digitalWrite(LED_Under, LOW);
    digitalWrite(LED_Air_pico, LOW);
    digitalWrite(LED_Air_xiao, LOW);
    digitalWrite(LED_GPS, LOW);
    digitalWrite(LED_SD, LOW);
    delay(500);
  }
  Serial.println("DEBUG MODE Enabled");
#endif  //DEBUG_MODEが有効ならば

  SDP31_init();
  Serial.println("SDP init done");
  AS5600_init();
  Serial.println("AS5600x2 setup done");
  BMP3XX_init();
  Serial.println("BMP setup done");

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

    read_bmp_air();  // BMP390 気圧・気温読み取り

    read_AS5600();  // AS5600読み取り AoA, AOS

    read_SDP();  // SDP31差圧読み取り．対気速度

    // GPSは10Hzつまり100msに一回読む．
    static int gps_counter = 0;
    if (gps_counter > 10) {
      read_gps();  // GPS読み取り
      gps_counter = 0;
    }
    gps_counter++;


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

    // 機体下・胴体桁読み取り
    receiveLog();

    // 高度計算．機体下・胴体桁電装の値を使うからUART受信後に計算
    calculate_altitude();

    // 対気速度計算．SDPとBMPの値を使うからUART受信後に計算
    calculate_airspeed();

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
