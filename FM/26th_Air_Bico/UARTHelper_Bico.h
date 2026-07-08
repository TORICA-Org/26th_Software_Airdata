#pragma once

#include "parameters.h"
#include <TORICA_ICS.h>
#include <TORICA_UART.h>

// `extern`宣言すれば`TransmitUART.h`をインクルードしたファイルで使えるようになる
extern SerialPIO Serial_ESP;
extern SerialPIO Serial_fslg; // 胴体桁基板(fslg)との通信
extern SerialPIO Serial_Under;

// 関数のプロトタイプ宣言
void initUART();
void transmitHeader();
void transmitLog(int);
void transmitLog_for_fslg(int);
void receiveLog();
