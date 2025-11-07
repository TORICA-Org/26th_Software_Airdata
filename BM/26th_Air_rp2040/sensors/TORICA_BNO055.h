// BNO055
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BNO055.h>

extern Adafruit_BNO055 *TORICA_bno;

void TORICA_BNO055_init(TwoWire &wire, uint8_t addr = 0x28);


Adafruit_BNO055* TORICA_BNO055_get();

void TORICA_BNO055_rollpitchyaw(float &roll, float &pitch, float &yaw);

void TORICA_BNO055_getCalibration(uint8_t &sys, uint8_t &gyro, uint8_t &accel, uint8_t &mag);

void TORICA_BNO055_acc(float &accx, float &accy, float &accz);

void TORICA_BNO055_quaternion(float &qw, float &qx, float &qy, float &qz);