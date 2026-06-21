/*---------------------------------------------------------

このファイルの役割：BicoのPin配置定義
最終更新日：2026/01/26 17:26
更新内容：ファイル作成

---------------------------------------------------------*/

#pragma once // インクルードガード（複数回読み込まれないようにする）

//ピン宣言用
//LED設定
extern const int LED_ICS;
extern const int LED_Under;
extern const int LED_Air_pico;
extern const int LED_Air_xiao;
extern const int LED_GPS;
extern const int LED_SD;

//UARTピン設定
extern const int Serial_ICS_TX;
extern const int Serial_ICS_RX;
extern const int Serial_GPS_TX;
extern const int Serial_GPS_RX;
extern const int Serial_Air_xiao_TX;
extern const int Serial_Air_xiao_RX;
extern const int Serial_Under_TX;
extern const int Serial_Under_RX;
extern const int Serial_fslg_TX;
extern const int Serial_fslg_RX;

//I2Cピン設定
extern const int bico_I2C0_SDA;
extern const int bico_I2C0_SCL;
extern const int bico_I2C1_SDA;
extern const int bico_I2C1_SCL;


//SD用ピン設定
extern const int SD_CS;
extern const int SD_SCK;
extern const int SD_MOSI;
extern const int SD_MISO;