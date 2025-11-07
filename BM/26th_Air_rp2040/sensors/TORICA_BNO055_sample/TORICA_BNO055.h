// BNO055 interface header
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BNO055.h>

extern Adafruit_BNO055 *TORICA_bno;

void TORICA_BNO055_init(TwoWire &wire, uint8_t addr = 0x28);


Adafruit_BNO055 *TORICA_BNO055_get();

void TORICA_BNO055_getData(float &roll, float &pitch, float &yaw);
