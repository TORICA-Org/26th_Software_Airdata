/*---------------------------------------------------------

このファイルの役割：BNO055初期化動作など
最終更新日：2026/02/03 13:26
更新内容：ファイル作成

---------------------------------------------------------*/

#include <Arduino.h>
#include "BNO055.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "parameters.h"

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

bool BNO055_init(void){
  if(!bno.begin()){
    #ifdef DEBUG_MODE
    Serial.println("no BNO055 detected");
    #endif
    while(1);
    return false;
  }
  bno.setExtCrystalUse(true);
  return true;
}

void read_BNO(void){
  // オイラー角（roll,pitch,yaw）の取得
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  data_air_bno_yaw = euler.x();   // yaw角
  data_air_bno_roll = euler.y();  // roll角
  data_air_bno_pitch = euler.z(); // pitch角
  
  // クォータニオンを取得
  imu::Quaternion quat = bno.getQuat(); 
  data_air_bno_qw = quat.w(); 
  data_air_bno_qx = quat.x(); 
  data_air_bno_qy = quat.y(); 
  data_air_bno_qz = quat.z(); 

  // 加速度の取得
  imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  data_air_bno_accx_mss = accel.x(); // x方向の加速度
  data_air_bno_accy_mss = accel.y(); // y方向の加速度
  data_air_bno_accz_mss = accel.z(); // z方向の加速度
  
}