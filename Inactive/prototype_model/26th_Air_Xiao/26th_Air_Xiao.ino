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

TaskHandle_t thp[1];  // マルチスレッドのタスクハンドル格納用


// デバッグ用タスクマネージャー
void printTaskStats() {
  // 統計情報を格納するバッファ
  char statsBuffer[1024];
  
  // FreeRTOSのランタイム統計を取得
  vTaskGetRunTimeStats(statsBuffer);
  
  Serial.println("=========================================");
  Serial.println("Task Name       Abs Time (us)   % Time");
  Serial.println("=========================================");
  Serial.println(statsBuffer);
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  // 内蔵LEDを出力モードに設定

  Serial.begin(115200);  // デバッグ用にパリティはいらないかな...ってか使えない気がする
  Serial.print("loading...\n\n");

  // Core1のタスク初期化
  setupSDandUART(); // SDとUARTの初期化

  // Core0のタスク初期化
  init_PowerChecker(); // 電流電圧計の初期化
  initSerialWeb(); // SerialWebの初期化

  // 内蔵LEDを点滅
  delay(100);
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_BUILTIN, HIGH);  // 内蔵LED ON
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);  // 内蔵LED OFF
    delay(1000);
  }


  xTaskCreatePinnedToCore(Core0_Task, "Core0_Task", 4096, NULL, 3, &thp[0], 0);

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

  xTaskCreatePinnedToCore(Core1_Task, "Core1_Task", 4096, NULL, 5, &thp[1], 1);
}




// Core0で行う処理
void Core0_Task(void *args) {
  TickType_t xLastWakeTime = xTaskGetTickCount();     // タスクの開始時間を取得
  const TickType_t xFrequency = pdMS_TO_TICKS(1000);  // 1000ms周期

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
    printTaskStats();
  }
}


// FreeRTOSにタスクを管理してもらうので，loop()内は空
void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}



// Core1で行う処理．
void Core1_Task(void *args) {
  TickType_t xLastWakeTime = xTaskGetTickCount();     // タスクの開始時間を取得
  const TickType_t xFrequency = pdMS_TO_TICKS(10);  // 10ms周期

  while (1) {  // ループさせたいので無限ループにする．FreeRTOSの仕様

    vTaskDelayUntil(&xLastWakeTime, xFrequency);  // vTaskDelayUntil()は指定した周期でタスクを実行するための関数．

    processCore1_UARTtoSD(); // UART受信とSD書き込みを行うタスク
  }
}