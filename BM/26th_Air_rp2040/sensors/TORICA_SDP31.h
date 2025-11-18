// SDP810用ヘッダーファイル
#pragma once

#include <Wire.h>
#include <SensirionI2CSdp.h>


extern SensirionI2CSdp *TORICA_sdp;


bool TORICA_SDP31_init(TwoWire &wire, uint8_t addr);


SensirionI2CSdp* TORICA_SDP31_get();

float TORICA_SDP31_getdifferentialPressure_Pa(void);