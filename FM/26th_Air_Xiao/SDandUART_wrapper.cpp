#include "SDandUART_wrapper.h"
#include "SD_Air_xiao.h"
#include "UARTHelper_air_xiao.h"
#include "SerialWebHelper.h"
#include <TORICA_UART.h>

QueueHandle_t uartQueue = NULL;  // UART受信データをCore0に送るためのキュー
QueueHandle_t sdQueue = NULL;    // SD書き込み用キュー

extern TORICA_UART Bico_UART;    // UARTHelper_air_xiao.cppで定義されているBico_UARTを外部参照


// ----------------------------------------------------
// 初期設定 (setup() 内で 1 回だけ呼び出す)
// ----------------------------------------------------
void setupSDandUART() {
  // キューを作成．100Hzで10個，つまり100ms分の遅延を吸収するバッファを確保．確保しすぎるとメモリが足りなくなる．
  uartQueue = xQueueCreate(3, sizeof(UARTData)); // SerialWebのほうは優先度低くていい
  sdQueue = xQueueCreate(5, sizeof(UARTData));   // SDのほうを優先．

  initSD();       // SD初期化
  flashHeader();  // csvヘッダー書き込み
  initUART();     // UART初期化
}


// ----------------------------------------------------
// Core0用処理：キューからデータ（バッファの中身）を受け取って文字列解析し，Webへ送信する
// ----------------------------------------------------
void processCore0_ParseAndWeb() {
  UARTData rxData;

  // キューからデータを受信 (最大100ms待つ)
  if (xQueueReceive(uartQueue, &rxData, pdMS_TO_TICKS(100))) { 
    // 受信したテキストバッファを各データに分解
    int parsed_num = Bico_UART.parseBuffer(rxData.text);
    
    // 正しいデータ項目数（54個）が揃っている場合のみ，ログデータを展開
    if (parsed_num == BICO_DATA_NUM) {
      extractLogData(parsed_num);
    }
  }

  // データが受信できなくても，電流・電圧計のWeb送信は継続する
  sendSerialWeb();
}


// ----------------------------------------------------
// Core1用処理(1)：UARTからデータを受信し，SDカード用とWeb用のキューに送る
// ----------------------------------------------------
void processCore1_ListenUART() {
  static int one_second_counter = 0;
  static UARTData txData;

  // UARTにデータが届いているか確認
  if (Bico_UART.listenUART()) {
    // listenUARTは末尾の'\n'を'\0'に書き換える仕様なので，
    // SDカード書き込み用に末尾に'\n'を付け直してコピーする
    snprintf(txData.text, sizeof(txData.text), "%s\n", Bico_UART.buff);

    // 完成したデータをSDカード書き込み用キューへ送信
    xQueueSend(sdQueue, &txData, 0);

    // 1秒（この関数は100Hzつまり10ms間隔で動くの1秒=25回）に1回，Core0（Web送信タスク）用のキューへ送信
    one_second_counter++;
    if (one_second_counter >= 25) {
      xQueueSend(uartQueue, &txData, 0);
      one_second_counter = 0;  // カウンターをリセット
    }
  }

  // Web側からリセット信号を受け取った場合の処理
  if (RESET_SIG == true) {
    snprintf(txData.text, sizeof(txData.text), "\nRESET\n");
    xQueueSend(sdQueue, &txData, 0);
    RESET_SIG = false;
  }
}


// ----------------------------------------------------
// Core1用処理(2)：SDカード用キューからデータを受け取ってSDへ書き込む
// ----------------------------------------------------
void processCore1_WriteSD() {
  static UARTData rxData;

  // SDカード用のキューからデータを取り出す (待機時間なし)
  if (xQueueReceive(sdQueue, &rxData, 0)) {
    // 内部バッファへデータを追記
    writeBufToSD(rxData.text);

    // 5回受信ごと (約50msごと) に，物理的にSDカードへ書き出す
    static int flash_counter = 0;
    flash_counter++;
    if (flash_counter >= 5) {
      writeSD();
      flash_counter = 0;  // カウンターをリセット
    }
  }
}