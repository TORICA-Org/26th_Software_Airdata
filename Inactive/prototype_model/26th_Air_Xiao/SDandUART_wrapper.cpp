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


// Core1でUARTを受信し，Core0に送るタスク
void sendUARTbuff(void *args) {
  UARTData txData;

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
  // if (xQueueReceive(uartQueue, &rxData, portMAX_DELAY)) {
    if (xQueueReceive(uartQueue, &rxData, pdMS_TO_TICKS(100)/* 100ms待ってもデータが来なかったらタイムアウト */ ) ){ 
    // バッファを各データに分解
    int parsed_num = Bico_UART.parseBuffer(rxData.text);
    // 53個揃っているかチェック
    if (parsed_num == 53) {
      extractLogData(parsed_num);  // 53個入っているかの確認はextractLogData()内でも行われているので，正直意味はないね
    }
  }

  sendSerialWeb();
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

    // 1秒 (25Hz周期なので25回) カウントする
    one_second_counter++;
    if (one_second_counter >= 25) {
      writeSD();
      // 1秒に1回Core0へ送信
      xQueueSend(uartQueue, &txData, 0);
      one_second_counter = 0;  // カウンターをリセット
    }
  }
}