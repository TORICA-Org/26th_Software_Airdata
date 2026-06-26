/* エアデータ ESP32 Xiao用ピン定義ファイル */
#pragma once
#include <Arduino.h>

// Bico用UART
extern const uint8_t BICO_UART_RX; // RX
extern const uint8_t BICO_UART_TX; // TX

// SDカードのピン設定
extern const uint8_t SD_CS;
extern const uint8_t SD_SCK;
extern const uint8_t SD_MOSI;
extern const uint8_t SD_MISO;

// 電流電圧計
extern const uint8_t Power_Checker_CURRENT;
extern const uint8_t Power_Checker_VOLTAGE;

// SerialWeb用SSID/PASSWORD
constexpr char SSID[] = "SerialWeb";
constexpr char PASSWORD[] = "12345678";