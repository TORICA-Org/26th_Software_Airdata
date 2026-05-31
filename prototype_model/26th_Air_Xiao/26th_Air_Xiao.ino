#define DEBUG_MODE

#include <Arduino.h>
#include "parameters.h"
#include "SerialWebHelper.h"
#include "Air_xiao_config.h"
#include "SD_Air_xiao.h"
#include "UARTHelper_air_xiao.h"
#include "power_checker.h"

TaskHandle_t thp[1];  // マルチスレッドのタスクハンドル格納用

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  // 内蔵LEDを出力モードに設定

  // ESP32って標準でloop1()関数使えないんだってさ
  xTaskCreatePinnedToCore(loop1, "loop1", 4096, NULL, 3, &thp[0], 0);
  //xTaskCreatePinnedToCore()がスレッドの宣言です。
  //内容は([タスク名], "[タスク名]", [スタックメモリサイズ(4096or8192)],
  //      NULL, [タスク優先順位](1-24,大きいほど優先順位が高い)],
  //      [宣言したタスクハンドルのポインタ(&thp[0])], [Core ID(0 or 1)]);

  Serial.begin(115200);  // デバッグ用にパリティはいらないかな...ってか使えない気がする
  Serial.print("loading...\n\n");

  initSD(); // SD初期化
  flashHeader(); // csvヘッダー書き込み

  initUART();

  init_PowerChecker();

  initSerialWeb();

  delay(1000);
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);  // 内蔵LED ON
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);  // 内蔵LED OFF
    delay(1000);
  }
}




void core0_func100Hz(){
  static int send_counter = 0;
  send_counter++;
  if (send_counter == 100 /* 1秒に1回送信*/){ 
  sendSerialWeb();

  /* USBシリアル
  Serial.print("AoS_angle_deg:");
  Serial.print(data_air_AoS_angle_deg);
  Serial.print("\t");
  Serial.print("AoA_angle_deg:");
  Serial.print(data_air_AoA_angle_deg);
  Serial.print("\t");
  Serial.print("Roll:");
  Serial.print(data_fslg_bno_roll);
  Serial.print("\t");
  Serial.print("Pitch:");
  Serial.print(data_fslg_bno_pitch);
  Serial.print("\t");
  Serial.print("Yaw:");
  Serial.print(data_fslg_bno_yaw);
  Serial.print("\t");
  Serial.print("BMP_Temp:");
  Serial.print(data_air_bmp_temperature_deg);
  Serial.print("\t");
  Serial.print("BMP_Pres:");
  Serial.print(data_air_bmp_pressure_hPa);
  Serial.print("\t");
  Serial.print("BMP_Alt:");
  Serial.print(data_air_bmp_altitude_m);
  Serial.print("\t");
  Serial.print("Airspeed:");
  Serial.print(data_air_sdp_airspeed_ms);
  Serial.println();
  */

  send_counter = 0;  // カウンターを0にリセット

  }

}



/* 以下100Hz実行用．ESP32でハードウェアタイマー使う方法まだ知らないから一旦API使わない方法で． */

uint32_t last_time_core0 = 0;
void loop() {
  if (millis() - last_time_core0 >= 10){

    core0_func100Hz();

    last_time_core0 = millis(); // 最後の実行時間を更新
  }
}


uint32_t last_time_core1 = 0;
void loop1(void *args) {
  if (millis() - last_time_core1 >= 10){
    // 10msごとに実行する処理をここに書く

    receiveLog(); // UARTからのデータ受信＆受信データを変数と紐づけ

    static uint8_t flash_counter = 0;
    flashSD(flash_counter);
    flash_counter++;
    if (flash_counter > 3) {
      flash_counter = 0;
    }
    last_time_core1 = millis(); // 最後の実行時間を更新
    }
}