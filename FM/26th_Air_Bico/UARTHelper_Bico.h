#pragma once

#include "parameters.h"

// DMA送信を使用する場合はコメントアウトを解除します
#define USE_DMA
#include <TORICA_ICS.h>
#include <TORICA_UART.h>

// `extern`宣言すれば`TransmitUART.h`をインクルードしたファイルで使えるようになる
extern SerialPIO Serial_ESP;
extern SerialPIO Serial_fslg; // 胴体桁基板(fslg)との通信
extern SerialPIO Serial_Under;

// 関数のプロトタイプ宣言
void initUART();
void initUART_DMA();
void transmitHeader();
void transmitLog(int trans_mode);
void transmitLog_for_fslg(int trans_mode);

// 機器ごとの個別受信関数
void receiveUnderLog();
void receiveFslgLog();
void receiveIcsAngle();
void handleEspSignal();
