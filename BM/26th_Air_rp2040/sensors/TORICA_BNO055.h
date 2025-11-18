// BNO055
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BNO055.h>

extern Adafruit_BNO055 *TORICA_bno;

void TORICA_BNO055_init(TwoWire &wire, uint8_t addr = 0x28);


Adafruit_BNO055* TORICA_BNO055_get();

void TORICA_BNO055_rollpitchyaw(volatile float &roll, volatile float &pitch, volatile float &yaw);

void TORICA_BNO055_getCalibration(volatile uint8_t &sys, volatile uint8_t &gyro, volatile uint8_t &accel, volatile uint8_t &mag);

void TORICA_BNO055_acc(volatile float &accx, volatile float &accy, volatile float &accz);

void TORICA_BNO055_quaternion(volatile float &qw, volatile float &qx, volatile float &qy, volatile float &qz);