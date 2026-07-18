/*---  ESP32S3 Xiao用  ---*/
// Core0: マイコン内部のシステム処理，SerialWebの処理，電流電圧計
// Core1: UART受信とSD書き込み
/*------------------------*/

#define DEBUG_MODE

#include <Arduino.h>
#include "parameters.h"
#include "SerialWebHelper.h"
#include "Air_xiao_config.h"
#include "SD_Air_xiao.h"
#include "UARTHelper_air_xiao.h"
#include "power_checker.h"
#include "SDandUART_wrapper.h"

TaskHandle_t thp[2];  // マルチスレッドのタスクハンドル格納用


// デバッグ用タスクマネージャー
void printTaskStats() {
  // タスク数が多いため、バッファを2048バイトに拡張
  static char statsBuffer[2048];
  
  // --------------------------------------------------
  // 1. CPU使用率の統計（既存の処理）
  // --------------------------------------------------
  vTaskGetRunTimeStats(statsBuffer);
  
  Serial.println("\n=========================================");
  Serial.println(" [CPU Time Stats]");
  Serial.println("Task Name       Abs Time (us)   % Time");
  Serial.println("-----------------------------------------");
  Serial.print(statsBuffer);

  // --------------------------------------------------
  // 2. タスクごとのメモリ（スタック空き容量）の統計 ★追加
  // --------------------------------------------------
  vTaskList(statsBuffer);
  
  Serial.println("-----------------------------------------");
  Serial.println(" [Task Memory Stats]");
  Serial.println("Task Name       State  Priority  MinFreeStack(B) Num");
  Serial.println("-----------------------------------------");
  Serial.print(statsBuffer);

  // --------------------------------------------------
  // 3. マイコン全体の空きメモリ（ヒープ） ★追加
  // --------------------------------------------------
  Serial.println("-----------------------------------------");
  Serial.print(" [System Total] Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" Bytes");
  Serial.println("=========================================\n");
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  // 内蔵LEDを出力モードに設定

  Serial.begin(115200);  // デバッグ用にパリティはいらないかな...ってか使えない気がする
  Serial.print("loading...\n\n");

  // Core1のタスク初期化
  setupSDandUART(); // SDとUARTの初期化
  Serial.println("SD and UART init done");

  // Core0のタスク初期化
  init_PowerChecker(); // 電流電圧計の初期化
  Serial.println("PowerChecker init done");

  initSerialWeb(); // SerialWebの初期化
  Serial.println("SerialWeb init done.");

  pinMode(LED_BUILTIN, OUTPUT);
  // 内蔵LEDを点滅
  // delay(100);
  // for (int i = 0; i < 2; i++) {
  //   digitalWrite(LED_BUILTIN, HIGH);  // 内蔵LED ON
  //   delay(1000);
  //   digitalWrite(LED_BUILTIN, LOW);  // 内蔵LED OFF
  //   delay(1000);
  // }


  // xTaskCreatePinnedToCore(Core0_Task, "Core0_Task", 16384, NULL, 3, &thp[0], 0);

  /* xTaskCreatePinnedToCore(
  Core0_Task, // [1] 実行する関数名
  "Task_on_Core0", // [2] タスクに名付ける名前（デバッグ用）
  4096, // [3] スタックメモリサイズ
  NULL, // [4] 関数に渡す引数（なければNULL）
  2, // [5] タスクの優先度（数値が大きいほど優先される）
  &thp[0], // [6] タスクハンドル（不要ならNULL ）
  0 // [7] タスクを実行するコア番号（0または1）
  )
  */

  // xTaskCreatePinnedToCore(Core1_Task, "Core1_Task", 16384, NULL, 5, &thp[1], 1);

  Serial.print("Free Heap before task create");
  Serial.println(ESP.getFreeHeap());

  BaseType_t ret0 = xTaskCreatePinnedToCore(Core0_Task, "Core0_Task", 12288, NULL, 1, &thp[0], 0);
  if (ret0 != pdPASS) Serial.println("Core0 Task creation failed!");

  Serial.print("Free Heap before Core1: ");
  Serial.println(ESP.getFreeHeap());

  BaseType_t ret1 = xTaskCreatePinnedToCore(Core1_Task, "Core1_Task", 8192, NULL, 1, &thp[1], 1);
  if (ret1 != pdPASS) Serial.println("Core1 Task creation failed!");
  Serial.println("All setup done.");
}


// FreeRTOSにタスクを管理してもらうので，loop()内は空
void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}



// SerialWebが落ちたときに復活させる用
#include <WiFi.h>
#include <esp_wifi.h>
static int wifi_recovery_count = 0;
void checkAndRecoverWiFiAP() {
  // APモードの自分のIPアドレスを取得
  IPAddress apIP = WiFi.softAPIP();

  // もしIPアドレスが0.0.0.0になっていたらAPが死んでいると判定
  if (apIP == IPAddress(0, 0, 0, 0)) {
    Serial.println("[WARNING] Wi-Fi AP is DOWN! Trying to recover...");

    // 一旦Wi-Fiを完全にリセット
    WiFi.softAPdisconnect(true);
    vTaskDelay(pdMS_TO_TICKS(100));

    // 明示的にAPモードで再起動
    WiFi.mode(WIFI_AP);
    // 引数: (SSID, PASSWORD, チャンネル, 隠蔽フラグ, 最大接続数1)
    WiFi.softAP(SSID, PASSWORD, 1, 0, 1); 
    
    // 省電力設定OFF
    WiFi.setSleep(false);
    esp_wifi_set_ps(WIFI_PS_NONE);

    // Serial.print("[RECOVER] Wi-Fi AP restarted. New IP: ");
    // Serial.println(WiFi.softAPIP());

    wifi_recovery_count++;
    
    Serial.println(wifi_recovery_count);
  }
}


// Core0で行う処理
void Core0_Task(void *args) {
  TickType_t xLastWakeTime = xTaskGetTickCount();     // タスクの開始時間を取得
  const TickType_t xFrequency = pdMS_TO_TICKS(100);  // 100ms周期

  // while (1) {  // ループさせたいので無限ループにする．FreeRTOSの仕様

  //   vTaskDelayUntil(&xLastWakeTime, xFrequency);  // vTaskDelayUntil()は指定した周期でタスクを実行するための関数．
  //   extractLogData();
  //   sendSerialWeb();

  //   /* デバッグ用 */
  //   Serial.println(millis());
  //   printTaskStats(); // FreeRTOSのタスク統計情報を表示
  //   /* ここまで */

  // }

  while (1){
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    processCore0_ParseAndWeb();
    checkAndRecoverWiFiAP();

    static int debug_count = 0;
    debug_count++;
    if (debug_count > 10){
    // Serial.println("Core0 running");
    // printTaskStats();
    // Serial.printf("[DEBUG] MinFreeHeap: %lu B | Core0Stack: %u B | Core1Stack: %u B\n", ESP.getMinFreeHeap(), /* ヒープの過去最低残高 */ uxTaskGetStackHighWaterMark(thp[0]),               /* Core0タスクのスタック残り */ uxTaskGetStackHighWaterMark(thp[1])                /* Core1タスクのスタック残り */);
    debug_count = 0;
    }
  }
}


// Core1で行う処理．
void Core1_Task(void *args) {
  TickType_t xLastWakeTime = xTaskGetTickCount();     // タスクの開始時間を取得
  const TickType_t xFrequency = pdMS_TO_TICKS(10);  // 10ms周期

  while (1) {  // ループさせたいので無限ループにする．FreeRTOSの仕様

    vTaskDelayUntil(&xLastWakeTime, xFrequency);  // vTaskDelayUntil()は指定した周期でタスクを実行するための関数．

    processCore1_ListenUART(); // UART受信を行うタスク
    processCore1_WriteSD(); // SD書き込みを行うタスクを実行

    // SerialWebからの"RESET"信号受け取り
    static uint8_t reset_signal_count = 0;
    reset_signal_count++;
    if (reset_signal_count > 15){
      SerialWeb_detectRESET();
      reset_signal_count = 0;
    }
    // Serial.println("Core1 running");
  }
}