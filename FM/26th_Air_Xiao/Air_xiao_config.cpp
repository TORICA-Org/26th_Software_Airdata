/* Air ESP32 Xiao用ピン設定 */
#include "Air_xiao_config.h"
#include <Arduino.h>

// UART
const uint8_t BICO_UART_RX = 44;
const uint8_t BICO_UART_TX = 43;

// SDカードのピン設定
const uint8_t SD_CS = D0;
const uint8_t SD_SCK = D8;
const uint8_t SD_MOSI = D10;
const uint8_t SD_MISO = D9;

// 電流電圧計
const uint8_t Power_Checker_CURRENT = D1; // LT6106と接続
const uint8_t Power_Checker_VOLTAGE = D2; // 分圧回路
