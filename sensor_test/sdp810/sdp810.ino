#include <Wire.h>
#include <SensirionI2CSdp.h>
#include <math.h>

SensirionI2CSdp sdp;
uint16_t error;
char errorMessage[256];

float sdp_airspeed_ms = 0;
float sdp_differentialPressure_Pa = 0;
float sdp_temperature_deg = 0;

void setup() {
  // put your setup code here, to run once:
  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire.setClock(400000);
  Wire.begin();

  Serial.begin(115200);

  sdp.begin(Wire, SDP8XX_I2C_ADDRESS_0);
  uint32_t productNumber;
  uint8_t serialNumber[8];   //配列のサイズの指定
  uint8_t serialNumberSize = 8;
  sdp.stopContinuousMeasurement();
  error = sdp.readProductIdentifier(productNumber, serialNumber, serialNumberSize); //読み取れた場合は0を返し，読み取れなかった場合はエラコードを返す
  if(error)
  {
    Serial.print("Error trying to execute readProductIdentifier(): ");
    errorToString(error, errorMessage, 256); //エラーが起こった時にエラーコードを文字列に変換する
    Serial.println(errorMessage);
  }
  else{
    Serial.print("ProductNumber:");
    Serial.print(productNumber);
    Serial.print("\t"); //空白の追加
    Serial.print("SerialNumber:");
    Serial.print("0x");
    for (size_t i = 0; i < serialNumberSize; i++) {
      Serial.print(serialNumber[i], HEX);  //整数型を16進数に変換する
    }
    Serial.println();
  }

   error = sdp.startContinuousMeasurementWithDiffPressureTCompAndAveraging();  //エラーが起こった時にエラーコードを文字列に変換する

  if (error) {
    Serial.print(
    "Error trying to execute "
    "startContinuousMeasurementWithDiffPressureTCompAndAveraging(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  error = sdp.readMeasurement(sdp_differentialPressure_Pa, sdp_temperature_deg);  //測定値を読み取る　読み取れた場合は2つの値が返され，読み取れなかった場合にはエラーコードが返される

  if(error) {
    Serial.print("Error trying to execute readMeasuremennt():");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  else{
    //Serial.print("sdp_differentialPressure[Pa]:");
    //Serial.print(sdp_differentialPressure_Pa);
    //Serial.print("\t");
    //Serial.print("sdp_temperature_deg[℃]:");
    //Serial.print(sdp_temperature_deg);
    //Serial.println();
    sdp_airspeed_ms = sqrt(abs(2.0 * sdp_differentialPressure_Pa / (0.0034837 * 101325.0 / (sdp_temperature_deg + 273.5)))); //差圧測定の測定温度はそこまで正確ではないため，本番機ではDPS310から取得した温度の値を使って機体速度を出力する
    //Serial.print("sdp_airspeed_ms[m/s]");
    Serial.println(sdp_airspeed_ms);
  }

}