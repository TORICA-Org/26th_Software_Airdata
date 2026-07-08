#pragma once
#include <Arduino.h>
#include "SD_Air_xiao.h"
#include "UARTHelper_air_xiao.h"

struct UARTData {
    char text[1024]; // Core1からCore0に送信するためのデータを格納するためのバッファ
};

// 受信したログデータを構造体に引き渡す
// 前提：受信データが各変数に格納されていること．
void copyLogDataToSDQueue(void *args);

// SDとUARTのセットアップ用wrapper関数
void setupSDandUART();

void processCore0_ParseAndWeb();
void processCore1_UARTtoSD();


