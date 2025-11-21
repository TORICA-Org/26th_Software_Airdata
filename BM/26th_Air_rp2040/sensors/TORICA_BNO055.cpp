//BNO055
//DEBUG MODE

#include <Arduino.h>
#include "TORICA_BNO055.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>


Adafruit_BNO055 *TORICA_bno = nullptr;

bool TORICA_BNO055_init(TwoWire &wire, uint8_t addr){
    //もし既に初期化されていたらtrueを返す
    if(TORICA_bno) return true;

    // Create the instance. Use sensor ID 55 to match previous code.
    TORICA_bno = new Adafruit_BNO055(55, addr, &wire);
    if(!TORICA_bno){
      #ifdef DEBUG_MODE
        Serial.println("BNO055 allocation failed");
      #endif
      return false;
    }
  

    if(!TORICA_bno->begin()){
      #ifdef DEBUG_MODE
        Serial.println("BNO055 not detected");
      #endif
      delete TORICA_bno;
      TORICA_bno = nullptr;
      while(1);
    }
    TORICA_bno->setExtCrystalUse(true);
    return true;
}

Adafruit_BNO055* TORICA_BNO055_get(){
    return TORICA_bno;
}

bool TORICA_BNO055_rollpitchyaw(volatile float &roll, volatile float &pitch, volatile float &yaw){
    roll = pitch = yaw = 0.0f;
    if(!TORICA_bno) return false;

    imu::Vector<3> euler = TORICA_bno->getVector(Adafruit_BNO055::VECTOR_EULER);
    roll = euler.x();
    pitch = euler.y();
    yaw = euler.z();
    return true;
}




bool TORICA_BNO055_getCalibration(volatile uint8_t &system, volatile uint8_t &gyro, volatile uint8_t &accel, volatile uint8_t &mag){
  system = gyro = accel = mag = 0;
  if(!TORICA_bno) return false;

  // Adafruit API expects non-volatile uint8_t* pointers. Use temporaries
  // and then copy into the volatile references to avoid invalid pointer conversion.
  uint8_t s = 0, g = 0, a = 0, m = 0;
  TORICA_bno->getCalibration(&s, &g, &a, &m);
  system = s;
  gyro = g;
  accel = a;
  mag = m;

  return true;
}

bool TORICA_BNO055_acc(volatile float &accx, volatile float &accy, volatile float &accz){
  accx = accy = accz = 0.0f;
  if(!TORICA_bno) return false;

  imu::Vector<3> acceleration = TORICA_bno->getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  accx = acceleration.x();
  accy = acceleration.y();
  accz = acceleration.z();
  return true;
}

bool TORICA_BNO055_quaternion(volatile float &qw, volatile float &qx, volatile float &qy, volatile float &qz){
  qw = qx = qy = qz = 0.0f;
  if(!TORICA_bno) return false;

  imu::Quaternion quat = TORICA_bno->getQuat();
  qw = quat.w();
  qx = quat.x();
  qy = quat.y();
  qz = quat.z();
  return true;
}



