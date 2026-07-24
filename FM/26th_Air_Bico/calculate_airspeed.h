#pragma once

#include <Arduino.h>

extern float CALIB_A;
extern float CALIB_B;
extern float CALIB_C;

void calculate_airspeed(float diff_press_Pa, float temp_deg, float press_hPa);
float correct_airspeed(float raw_airspeed);