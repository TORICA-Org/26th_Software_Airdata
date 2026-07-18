#pragma once

#include <Arduino.h>
void calculate_airspeed(float diff_press_Pa, float temp_deg, float press_hPa);
float correct_airspeed(float raw_airspeed);