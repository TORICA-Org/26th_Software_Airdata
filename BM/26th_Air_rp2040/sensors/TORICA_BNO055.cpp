//BNO055
//DEBUG MODE

#include <Arduino.h>
#include "TORICA_BNO055.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// Define the global pointer (declared extern in the header).
Adafruit_BNO055 *TORICA_bno = nullptr;

void TORICA_BNO055_init(TwoWire &wire, uint8_t addr){
    // If already initialized, do nothing.
    if(TORICA_bno) return;

    // Create the instance. Use sensor ID 55 to match previous code.
    TORICA_bno = new Adafruit_BNO055(55, addr, &wire);
    if(!TORICA_bno){
      #ifdef DEBUG_MODE
        Serial.println("BNO055 allocation failed");
      #endif
      return;
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
}

Adafruit_BNO055* TORICA_BNO055_get(){
    return TORICA_bno;
}

void TORICA_BNO055_rollpitchyaw(float &roll, float &pitch, float &yaw){
    roll = pitch = yaw = 0.0f;
    if(!TORICA_bno) return;

    imu::Vector<3> euler = TORICA_bno->getVector(Adafruit_BNO055::VECTOR_EULER);
    roll = euler.x();
    pitch = euler.y();
    yaw = euler.z();
}


void TORICA_BNO055_getCalibration(uint8_t &sys, uint8_t &gyro, uint8_t &accel, uint8_t &mag){
    sys = gyro = accel = mag = 0;
    if(!TORICA_bno) return;

    TORICA_bno->getCalibration(&sys, &gyro, &accel, &mag);
}

void TORICA_BNO055_acc(float &accx, float &accy, float &accz){
    accx = accy = accz = 0.0f;
    if(!TORICA_bno) return;

    imu::Vector<3> acceleration = TORICA_bno->getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
    accx = acceleration.x();
    accy = acceleration.y();
    accz = acceleration.z();
}

void TORICA_BNO055_quaternion(float &qw, float &qx, float &qy, float &qz){
    qw = qx = qy = qz = 0.0f;
    if(!TORICA_bno) return;

    imu::Quaternion quat = TORICA_bno->getQuat();
    qw = quat.w();
    qx = quat.x();
    qy = quat.y();
    qz = quat.z();
}



