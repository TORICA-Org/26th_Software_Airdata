#pragma once

#include <Arduino.h>
#include "parameters.h"

extern const uint8_t BICO_DATA_NUM;

void initUART();
void receiveLog();

// ロジックと代入の分離用の関数宣言
LogData convertArrayToLogData(const float* uart_data);
void applyLogDataToGlobals(const LogData& data);

void extractLogData(int readnum);