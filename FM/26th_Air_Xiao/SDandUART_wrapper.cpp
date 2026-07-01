#include "SDandUART_wrapper.h"
#include "SD_Air_xiao.h"
#include "UARTHelper_air_xiao.h"
#include "SerialWebHelper.h"
#include <TORICA_UART.h>

QueueHandle_t uartQueue = NULL;  // UART受信データをCore0に送るためのキュー
QueueHandle_t sdQueue = NULL;    // SD書き込み用キュー

extern TORICA_UART Bico_UART;    // UARTHelper_air_xiao.cppで定義されているBico_UARTを外部参照


// この関数はsetup()内で呼び出す
void setupSDandUART() {
  // キューを作成．100Hzで50個，つまり500ms分の遅延を吸収するバッファを確保．
  uartQueue = xQueueCreate(50, sizeof(UARTData));
  sdQueue = xQueueCreate(50, sizeof(SDData));

  initSD();       // SD初期化
  flashHeader();  // csvヘッダー書き込み
  initUART();     // UART初期化
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
    if (parsed_num == 54) {
      extractLogData(parsed_num);  // 54個入っているかの確認はextractLogData()内でも行われているので，正直意味はないね
    }
  }

  // データ受け取れなくてもとりあえずSerialWebは動かす．電流電圧見れなくなるし．
  sendSerialWeb();
}




void processCore1_ListenUART() {
  static int one_second_counter = 0;
  UARTData txData;

  if (Bico_UART.listenUART()) {

    // TORICA_UART.listenUART() は末尾の'\n'を'\0'に書き換えてしまう仕様なので、
    // SD用Queueに送るために，改めて末尾に'\n'を付け直してtxDataに入れる
    snprintf(txData.text, sizeof(txData.text), "%s\n", Bico_UART.buff); // TORICA_UART内に保存されたバッファをtxData.textに保存し末尾を\nに変更

    // 完成した1行をSDカードキューに送る
    xQueueSend(sdQueue, &txData, 0);

    // SerialWeb用にデータをCore0に送る (1秒に1回)
    // 1秒 (25Hz周期なので25回) カウントする
    one_second_counter++;
    if (one_second_counter >= 25) {
      // 1秒に1回Core0へ送信
      xQueueSend(uartQueue, &txData, 0);
      xQueueSend(sdQueue, &txData, 0); // SDキューにも送る
      one_second_counter = 0;  // カウンターをリセット
    }
  }
}

void processCore1_WriteSD() {
  UARTData rxData;

  // SDカードキューからデータを受信
  if (xQueueReceive(sdQueue, &rxData, pdMS_TO_TICKS(30)/* 30ms待ってもデータが来なかったらタイムアウト */)) {
    // SDバッファに書き込み
    writeBufToSD(rxData.text);

    static int flash_counter = 0;
    flash_counter++;
    if (flash_counter >= 5) {  // 50msに1回SDに書き込む
      writeSD();  // SDに書き込み
      flash_counter = 0;  // カウンターをリセット
    }
  }
}