/*---------------------------------------------------------

このファイルの役割：BMP390の初期化・値取得
最終更新日：2026/04/11 00:42
更新内容：read_bmp_fslg()作成
注) 各電装部において使用しない関数はコメントアウトすること．

---------------------------------------------------------*/

#pragma once

#include <Arduino.h>
#include <Wire.h>

struct BmpData {
    float pressure_hPa;
    float temperature_deg;
};

bool BMP3XX_init(TwoWire* wire, uint8_t address);
BmpData read_bmp();