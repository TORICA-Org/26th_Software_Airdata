#ifndef TORICA_AS5600_H
#define TORICA_AS5600_H

#include <Arduino.h>
#include <Wire.h>
#include <AS5600.h>


#ifdef __cplusplus
extern "C" {
#endif

bool TORICA_AS5600_AoS_init(TwoWire &wire, uint8_t address = 0x36);
bool TORICA_AS5600_AoA_init(TwoWire &wire, uint8_t address = 0x36);
float TORICA_AS5600_getAoS(void);
float TORICA_AS5600_getAoA(void);

#ifdef __cplusplus
}
#endif

#endif // TORICA_AS5600_H