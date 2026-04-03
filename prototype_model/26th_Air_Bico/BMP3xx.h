/*---------------------------------------------------------

このファイルの役割：BMP390の初期化・値取得
最終更新日：2026/02/20 16:53
更新内容：read_bmp_air()，read_bmp_under()作成

---------------------------------------------------------*/


#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP3XX.h>


//センサー初期化用
bool BMP3XX_init(void);

void read_bmp_air(void);

void read_bmp_under(void);


//float BMP3XX_getTemperature_deg(void);

//float BMP3XX_getPressure_hPa(void);