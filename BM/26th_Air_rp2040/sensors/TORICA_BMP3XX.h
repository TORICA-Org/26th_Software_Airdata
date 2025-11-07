//BMP3XX用ヘッダーファイル
#pragma once
class Adafruit_BMP3XX;

extern Adafruit_BMP3XX *TORICA_bmp;

//関数定義
bool TORICA_BMP3XX_init(TwoWire &wire, uint8_t addr = 0x77);
Adafruit_BMP3XX* TORICA_BMP3XX_get();
float TORICA_BMP3XX_getTemperature(void);
float TORICA_BMP3XX_getPressure(void);