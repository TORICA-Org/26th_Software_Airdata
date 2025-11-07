// SDP810用ヘッダーファイル
#pragma once

#include <Wire.h>
#include <SensirionI2CSdp.h>


extern SensirionI2CSdp *TORICA_sdp;


bool TORICA_SDP810_init(TwoWire &wire, uint8_t addr);


SensirionI2CSdp* TORICA_SDP810_get();


float TORICA_SDP810_getdifferentialPressure(void);
