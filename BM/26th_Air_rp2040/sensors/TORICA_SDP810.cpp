// SDP810用コード
// DEBUG MODE

      #include <Arduino.h>
      #include "TORICA_SDP810.h"
      #include <Wire.h>
      #include <SensirionI2CSdp.h>

      // Global pointer (declared extern in the header)
      SensirionI2CSdp *TORICA_sdp = nullptr;
      static uint16_t error;
      static char errorMessage[256];

      bool TORICA_SDP810_init(TwoWire &wire, uint8_t addr){
          // If already initialized, do nothing
          if(TORICA_sdp) return true;

          // Allocate the object
          TORICA_sdp = new SensirionI2CSdp();
          if(!TORICA_sdp){
            #ifdef DEBUG_MODE
              Serial.println("SDP810 allocation failed");
            #endif
            return false;
          }

          TORICA_sdp->begin(wire, addr);
          uint32_t productNumber;
          uint8_t serialNumber[8];   //配列のサイズの指定
          uint8_t serialNumberSize = 8;
          TORICA_sdp->stopContinuousMeasurement();
          error = TORICA_sdp->readProductIdentifier(productNumber, serialNumber, serialNumberSize); //読み取れた場合は0を返し，読み取れなかった場合はエラコードを返す
          if(error){
            errorToString(error, errorMessage, 256); //エラーが起こった時にエラーコードを文字列に変換する
            #ifdef DEBUG_MODE
              Serial.print("Error trying to execute readProductIdentifier(): ");
              Serial.println(errorMessage);
            #endif
            return false;
          }else{
            #ifdef DEBUG_MODE
              Serial.print("ProductNumber:");
              Serial.print(productNumber);
              Serial.print("\t"); //空白の追加
              Serial.print("SerialNumber:");
              Serial.print("0x");
              for (size_t i = 0; i < serialNumberSize; i++) {
                  Serial.print(serialNumber[i], HEX);  //整数型を16進数に変換する
              }
               Serial.println();
            #endif
            // continue to start measurement
          }

         error = TORICA_sdp->startContinuousMeasurementWithDiffPressureTCompAndAveraging();  //エラーが起こった時にエラーコードを文字列に変換する

        if (error) {
          errorToString(error, errorMessage, 256);
          #ifdef DEBUG_MODE
            Serial.print(
            "Error trying to execute "
            "startContinuousMeasurementWithDiffPressureTCompAndAveraging(): ");
            Serial.println(errorMessage);
          #endif
          return false;
        }

        return true;
      }

      SensirionI2CSdp* TORICA_SDP810_get(){
          return TORICA_sdp;
      }

      float TORICA_SDP810_getdifferentialPressure(void){
          float differentialPressure = 0.0f;
          float temperature = 0.0f;

          if(!TORICA_sdp) return 0.0f;

          error = TORICA_sdp->readMeasurement(differentialPressure, temperature);  // 測定値を読み取る
          if(error) {
          #ifdef DEBUG_MODE
              Serial.print("SDP810 error");
              errorToString(error, errorMessage, 256);
              Serial.println(errorMessage);
          #endif
          }

          //差圧のみ返す
          return differentialPressure;
      }