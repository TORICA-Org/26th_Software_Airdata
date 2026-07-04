/*---------------------------------------------------------

このファイルの役割：Bicoのピン配置定義
最終更新日：2026/01/26 17:26
更新内容：ファイル作成

---------------------------------------------------------*/

#include <Arduino.h> // Arduinoの基本的な関数を使えるようにする
#include "Bico_config.h"

//ピン宣言用
//Bico air_rp2040用
// LED設定
const int LED_ICS = 8;
const int LED_Under = 9;
const int LED_Air_pico = 10;
const int LED_Air_xiao = 22;
const int LED_GPS = 23;
const int LED_SD = 24;

// UARTピン設定
const int Serial_ICS_TX = 0;  // 使ってない架空のピンにする．こちら側から送信は行わないので
const int Serial_ICS_RX = 1;
const int Serial_GPS_TX = 14; // Bico TX <-> GPS RX
const int Serial_GPS_RX = 13; // Bico RX <-> GPS TX
const int Serial_Air_xiao_TX = 4;
const int Serial_Air_xiao_RX = 5;
const int Serial_Under_TX = 2;
const int Serial_Under_RX = 3;
const int Serial_fslg_TX = 12;
const int Serial_fslg_RX = 18;


// I2Cピン設定
const int bico_I2C0_SDA = 20;
const int bico_I2C0_SCL = 21;
const int bico_I2C1_SDA = 26;
const int bico_I2C1_SCL = 27;

// リセットボタン
const int RESET_BTN = 11;