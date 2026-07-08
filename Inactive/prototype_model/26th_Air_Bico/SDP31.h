/*---------------------------------------------------------

このファイルの役割：SDP810の初期化＆値取得用コード
最終更新日：2026/01/26 19:00
更新内容：ファイル作成

---------------------------------------------------------*/
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2CSdp.h>



extern SensirionI2CSdp sdp;

bool SDP31_init(void);

void read_SDP(void);

//float SDP31_getdifferentialPressure_Pa(void);


