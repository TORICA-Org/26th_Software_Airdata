#ifndef TORICA_TSD20_H
#define TORICA_TSD20_H

#pragma once

#include <Arduino.h> 

// TSD20から読み取り、距離[m]を返す
float TORICA_TSD20_getDistance_m(Stream &serial);

#endif // TORICA_TSD20_H