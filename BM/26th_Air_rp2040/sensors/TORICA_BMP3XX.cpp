//BMP3XX用コード
//DEBUG MODE

#include <Arduino.h>
#include "TORICA_BMP3XX.h"
#include <Wire.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_Sensor.h>

Adafruit_BMP3XX *TORICA_bmp = nullptr;

bool TORICA_BMP3XX_init(TwoWire &wire, uint8_t addr = 0x77) {
    if (TORICA_bmp) return true;

    TORICA_bmp = new Adafruit_BMP3XX();
    if (!TORICA_bmp) {
        #ifdef DEBUG_MODE
            Serial.println("BMP3XX allocation failed");
        #endif
        return false;
    }

    if (!TORICA_bmp->begin_I2C(addr, &wire)) {
        #ifdef DEBUG_MODE
            Serial.println("Could not find a valid BMP3 sensor, check wiring!");
        #endif
        delete TORICA_bmp;
        TORICA_bmp = nullptr;
        return false;
    }

    // オーバーサンプリング設定
    TORICA_bmp->setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    TORICA_bmp->setPressureOversampling(BMP3_OVERSAMPLING_4X);
    // IIRフィルタ設定
    TORICA_bmp->setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    // 12.5Hzで出力
    TORICA_bmp->setOutputDataRate(BMP3_ODR_12_5_HZ);

    return true;
}

Adafruit_BMP3XX* TORICA_BMP3XX_get(){
    return TORICA_bmp;
}

float TORICA_BMP3XX_getTemperature(void) {
    if (!TORICA_bmp) return 0.0f;
    return TORICA_bmp->temperature;
}

float TORICA_BMP3XX_getPressure(void) {
    if (!TORICA_bmp) return 0.0f;
    return TORICA_bmp->pressure / 100.0f;
}