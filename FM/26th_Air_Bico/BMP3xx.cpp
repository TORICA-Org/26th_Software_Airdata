/*---------------------------------------------------------

このファイルの役割：BMP390の初期化・値取得
最終更新日：2026/04/11 00:42
更新内容：read_bmp_fslg()作成
注) 各電装部において使用しない関数はコメントアウトすること．

---------------------------------------------------------*/

#include "BMP3xx.h"
#include <Adafruit_BMP3XX.h>

static Adafruit_BMP3XX bmp;

bool BMP3XX_init(TwoWire* wire, uint8_t address) {
    if (!bmp.begin_I2C(address, wire)) {
        #ifdef DEBUG_MODE
        Serial.println("Could not find a valid BMP3 sensor, check wiring!");
        #endif
        return false;
    }
    
    // Set up oversampling and filter initialization
    bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    bmp.setOutputDataRate(BMP3_ODR_50_HZ);

    return true;
}

BmpData read_bmp() {
    BmpData data = {0.0f, 0.0f};

    if (bmp.performReading()) {
        data.pressure_hPa = bmp.pressure / 100.0f; // 気圧をhPaで表現
        data.temperature_deg = bmp.temperature; // 温度を℃で返す
    } else {
        #ifdef DEBUG_MODE
        Serial.println("Failed to reading BMP3 :(");
        #endif
    }
    return data;
}
