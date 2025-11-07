#define DEBUG_MODE //デバッグモード

#include <Arduino.h> 
#include "parameters.h" 
#include "functions.h" // 使用したい機能が書かれたヘッダーファイル(.h)をインクルード

//UARTの宣言
#define SerialAir_xiao Serial1;
#define SerialUnder Serial2;
SerialPIO Serial_ICS(10,11); //ICS基盤用
SerialPIO SerialGPS(2,3); //GPS用


//ライブラリ読み込みと初期化とか
//GPS
#include <TinyGPSPlus.h>
TinyGPSPlus gps;
//TORICA_UARTインスタンス化
#include <TORICA_UART.h>
TORICA_UART Under_UART(SerialUnder);
TORICA_UART Air_xiao_UART(SerialAir_xiao);
//TORICA_ICS初期化
#include <TORICA_ICS.h>
TORICA_ICS ics(&Serial_ICS);
//I2Cパッケージ
#include <Wire.h>




void setup(){
  Serial.begin(460800);
  LED_init(); //LED初期化

  //DEBUG_MODEが有効の時のみ実行
  #ifdef DEBUG_MODE
    init_delay_10sec(); //USBケーブルを差したときの起動猶予．処理を止めたいのでdelay関数使う
    Serial.println("Debug Mode Enabled");
  #endif //DEBUG_MODE
}

void loop () {}