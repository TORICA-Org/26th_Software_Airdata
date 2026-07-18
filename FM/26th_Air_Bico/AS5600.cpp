/*---------------------------------------------------------

このファイルの役割：AS5600の値読み取り
最終更新日：2026/01/27 17:34
更新内容：ファイル作成

---------------------------------------------------------*/

#include "AS5600.h"
#include <Wire.h>
#include <AS5600.h>

static AS5600 AoA(&Wire);  // I2C0を使用
static AS5600 AoS(&Wire1); // I2C1を使用

bool AS5600_init() {

  #ifdef DEBUG_MODE
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);
  Serial.println();
  #endif

  AoA.setDirection(AS5600_CLOCK_WISE);
  AoS.setDirection(AS5600_CLOCK_WISE);
  
  #ifdef DEBUG_MODE
  if(AoS.isConnected() == 0){
    Serial.println("AoS error");
  } else if(AoS.isConnected() == 1){
    Serial.println("AoS OK!");
  }

  if(AoA.isConnected() == 0){
    Serial.println("AoA error");
  }
  else if(AoA.isConnected() == 1){
    Serial.println("AoA OK!");
  }
  #endif

  return (AoA.isConnected() && AoS.isConnected());
}

AS5600Data read_AS5600() {
  AS5600Data data = {0.0f, 0.0f};

  int raw_AoA = AoA.readAngle();
  int raw_AoS = AoS.readAngle();

  if (raw_AoA > 2048) raw_AoA -= 4096;
  if (raw_AoS > 2048) raw_AoS -= 4096;

  data.AoA_deg = (raw_AoA * 360.0f) / 4096.0f;
  data.AoS_deg = (raw_AoS * 360.0f) / 4096.0f;

  return data;
}