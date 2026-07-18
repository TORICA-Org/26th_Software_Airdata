/*---------------------------------------------------------

このファイルの役割：SDP31の初期化＆値取得用コード
最終更新日：2026/01/26 19:00
更新内容：ファイル作成

---------------------------------------------------------*/

#include <Arduino.h>
#include <SensirionI2CSdp.h>
#include <Wire.h>
#include "SDP31.h"
#include "parameters.h"

static SensirionI2CSdp sdp;

bool SDP31_init(TwoWire* wire, uint8_t address) {
    uint16_t error;
    char errorMessage[256];

    sdp.begin(*wire, address);

    uint32_t productNumber;
    uint8_t serialNumber[8];
    uint8_t serialNumberSize = 8;

    sdp.stopContinuousMeasurement();

    error = sdp.readProductIdentifier(productNumber, serialNumber,
                                      serialNumberSize);
    if (error) {
        errorToString(error, errorMessage, 256);
        #ifdef DEBUG_MODE
        Serial.print("Error trying to execute readProductIdentifier(): ");
        Serial.println(errorMessage);
        #endif

        return false;
    } else {
      #ifdef DEBUG_MODE
        Serial.print("ProductNumber:");
        Serial.print(productNumber);
        Serial.print("\t");
        Serial.print("SerialNumber:");
        Serial.print("0x");
        for (size_t i = 0; i < serialNumberSize; i++) {
            Serial.print(serialNumber[i], HEX);
        }
        Serial.println();
      #endif
    }

    error = sdp.startContinuousMeasurementWithDiffPressureTCompAndAveraging();

    if (error) {
        #ifdef DEBUG_MODE
        Serial.print(
            "Error trying to execute "
            "startContinuousMeasurementWithDiffPressureTCompAndAveraging(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        #endif
        return false;
    }
    return true;
}

float read_SDP(void){
    uint16_t error;
    char errorMessage[256];

    float differentialPressure = 0.0f;
    float temperature = 0.0f;

    error = sdp.readMeasurement(differentialPressure, temperature);

    if (error) {
      #ifdef DEBUG_MODE
        Serial.print("Error trying to execute readMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
      #endif
        return 0.0f;
    }
    return differentialPressure;
}