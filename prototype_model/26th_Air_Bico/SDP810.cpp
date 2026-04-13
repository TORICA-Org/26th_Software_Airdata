/*---------------------------------------------------------

このファイルの役割：SDP810の初期化＆値取得用コード
最終更新日：2026/01/26 19:00
更新内容：ファイル作成

---------------------------------------------------------*/

#include <Arduino.h>
#include <SensirionI2CSdp.h>
#include <Wire.h>
#include "SDP810.h"
#include "parameters.h"

SensirionI2CSdp sdp;

bool SDP810_init(void) {
    uint16_t error;
    char errorMessage[256];

    sdp.begin(Wire, SDP8XX_I2C_ADDRESS_0);

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
        #endif DEBUG_MODE
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
      #endif DEBUG_MODE
    }

    error = sdp.startContinuousMeasurementWithDiffPressureTCompAndAveraging();

    if (error) {
        Serial.print(
            "Error trying to execute "
            "startContinuousMeasurementWithDiffPressureTCompAndAveraging(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        return false;
    }
    return true;
}

void read_SDP(void){
  uint16_t error;
    char errorMessage[256];

    // Read Measurement
    float differentialPressure;
    float temperature;

    error = sdp.readMeasurement(differentialPressure, temperature);

    if (error) {
      #ifdef DEBUG_MODE
        Serial.print("Error trying to execute readMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
      #endif DEBUG_MODE
       data_air_sdp_differentialPressure_Pa = 0.0; //もし読み取れなかったら
    } else {
      data_air_sdp_differentialPressure_Pa = differentialPressure;
    }
}

// float SDP810_getdifferentialPressure_Pa(void) {
//     uint16_t error;
//     char errorMessage[256];

//     // Read Measurement
//     float differentialPressure;
//     float temperature;

//     error = sdp.readMeasurement(differentialPressure, temperature);

//     if (error) {
//       #ifdef DEBUG_MODE
//         Serial.print("Error trying to execute readMeasurement(): ");
//         errorToString(error, errorMessage, 256);
//         Serial.println(errorMessage);
//       #endif DEBUG_MODE
//       return 0.0;
//     } else {
//       return differentialPressure;
//       /*----------------
//         Serial.print("DifferentialPressure[Pa]:");
//         Serial.print(differentialPressure);
//         Serial.print("\t");
//         Serial.print("Temperature[°C]:");
//         Serial.print(temperature);
//         Serial.println();
//       -----------------*/
//     }
// }