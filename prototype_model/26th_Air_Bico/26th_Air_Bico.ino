#define DEBUG_MODE  // デバッグモード

#include <Arduino.h>
#include "parameters.h"
#include "Bico_config.h"

// 各ファイル読み込み
#include "calculate_altitude.h"
#include "calculate_airspeed.h"
#include "AS5600.h"
#include "BMP3xx.h"
#include "SDP810.h"
#include "UARTHelper_Bico.h"


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

  Serial.begin(460800, SERIAL_8E1);  //DEBUG用USB-UART

  //ESP用・Under用UART初期化
  initUART();

  //SD内csv用ヘッダー送信
  transmitHeader();

  //Bico I2C0初期化動作
  Wire.setSDA(bico_I2C0_SDA);
  Wire.setSCL(bico_I2C0_SCL);
  Wire1.setSDA(bico_I2C1_SDA);
  Wire1.setSCL(bico_I2C1_SCL);
  Wire.begin();
  Wire1.begin();
  Wire.setClock(400000);
  Wire1.setClock(400000);


//USB接続時のために起動待機（7秒）
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


  SDP810_init();
  AS5600_init();
  BMP3XX_init();

  watchdog_enable(2000, 1);  // watchdogを有効化．
  /* 2000ms(=2s)経っても反応がない場合，
  システムが暴走したとみなして強制再起動 */

  // ハードウェアタイマー起動
  add_repeating_timer_ms(-10, core0_timer_callback, NULL, &core0_timer);
}


//CPU1のセットアップ
void setup1() {
  // ハードウェアタイマーの設定はコアごとに
  add_repeating_timer_ms(-10, core1_timer_callback, NULL, &core1_timer);
}


void loop() {
  if (core0_timer_triggered == true) {
    core0_timer_triggered = false;  // タイマーのフラグを戻す
    read_bmp_air();

    read_AS5600();

    read_SDP();


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

    //機体下読み取り
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
    //一通り送信(=transmit_countが4以上)したらカウントリセット
    if (transmit_count > 3) {
      transmit_count = 0;
    }

    core1_alive = true;  // core1生存フラグを立てる
  }
}
