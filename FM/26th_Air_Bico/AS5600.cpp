/*---------------------------------------------------------

このファイルの役割：AS5600の値読み取り
最終更新日：2026/01/27 17:34
更新内容：ファイル作成

---------------------------------------------------------*/

#pragma once // インクルードガード（複数回読み込まれないようにする）

#include "AS5600.h"
#include "AS5600.h"
#include "parameters.h"


//  Uncomment the line according to your sensor type
//AS5600L as5600;   //  use default Wire
AS5600 AoA(&Wire); //I2C0を使用
AS5600 AoS(&Wire1); //I2C1を使用


bool AS5600_init(void){

  #ifdef DEBUG_MODE
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);
  Serial.println();
  #endif DEBUG_MODE

  //AoA.begin(4);  //  set direction pin.
  //AoS.begin();


  //これどっちにするべきだろう・・・？
  AoA.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.
  AoS.setDirection(AS5600_CLOCK_WISE);
  
  #ifdef DEBUG_MODE
  //AoS
  if(AoS.isConnected() == 0){
    Serial.println("AoS error");
  } else if(AoS.isConnected() == 1){
    Serial.println("AoS OK!");
  }

  //AoA
  if(AoA.isConnected() == 0){
    Serial.println("AoA error");
  }
  else if(AoA.isConnected() ==1){
    Serial.println("AoA OK!");
  }
  #endif DEBUG_MODE

  if(AoA.isConnected() && AoS.isConnected()){
    return true; //両方とも接続成功の場合
  } else {
    return false; //どちらか又は両方が接続失敗の場合
  }

}


void read_AS5600(void){
  // .readAngle()で0～4096(12bitなので2^12)の値が返ってくる
  int raw_AoA = AoA.readAngle();
  int raw_AoS = AoS.readAngle();

  // 180°(2048)を超えていたら，4096を引いてマイナスにする
  if (raw_AoA > 2048) raw_AoA -= 4096;
  if (raw_AoS > 2048) raw_AoS -= 4096;

  data_air_AoA_angle_deg = (raw_AoA * 360.0) / 4096.0;
  data_air_AoS_angle_deg = (raw_AoS * 360.0) / 4096.0;
}


/*------
float AS5600_getAoS(void)
{
  return AoS.readAngle();
}

float AS5600_getAoA(void)
{
  return AoA.readAngle();
}

----------*/


//  -- END OF FILE --