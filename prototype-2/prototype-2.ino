#include <Arduino.h> 

//UARTの宣言
#define SerialAir_xiao Serial1
// #define SerialUnder Serial2
//SerialPIO SerialGPS(2,3); //GPS用

#include <Wire.h>

void setup() {

  Serial.begin(115200);
  SerialAir_xiao.setFIFOSize(1024);
  SerialAir_xiao.setTX(16);
  SerialAir_xiao.setRX(17);
  SerialAir_xiao.begin(115200, SERIAL_8N1);
  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire1.setSDA(26);
  Wire1.setSCL(27);
  Wire.begin();
  Wire1.begin();
  Wire.setClock(400000);
  Wire1.setClock(400000);

  int leds[] = {8, 9, 10, 22, 23, 24};
  for (int i = 0; i < sizeof(leds)/sizeof(leds[0]); i++) {
    pinMode(leds[i], OUTPUT);
    analogWrite(leds[i], 128);
  }
  
}

void loop() {
  
  for (int a = 0; a < 16; a++) {
    //Serial.printf("%02x  ", a);
    SerialAir_xiao.print(a,HEX);
    SerialAir_xiao.print(" ");
  }
  SerialAir_xiao.print("\n");

  int c = 0;
  for (int b = 0; b < 16; b++) {
    //SerialAir_xiao.printf("%02x  ", b);
    SerialAir_xiao.print(b,HEX);
    SerialAir_xiao.print(" ");
    for (int a = 0; a < 16; a++) {

      Wire.beginTransmission(c);
      int result = 0;
      result = Wire.endTransmission();
      if (result == 0) {
        
        //SerialAir_xiao.printf("%02x  ", c);
        SerialAir_xiao.print(c,HEX);
        SerialAir_xiao.print(" ");
      } else {
        SerialAir_xiao.print("--  ");
      }
      c++;
    }
    SerialAir_xiao.print("\nNEXT Wire1");
  }

  delay(1000);

  
  for (int a = 0; a < 16; a++) {
    //Serial.printf("%02x  ", a);
    SerialAir_xiao.print(a,HEX);
    SerialAir_xiao.print(" ");
  }
  SerialAir_xiao.print("\n");

  c = 0;
  for (int b = 0; b < 16; b++) {
    //SerialAir_xiao.printf("%02x  ", b);
    SerialAir_xiao.print(b,HEX);
    SerialAir_xiao.print(" ");
    for (int a = 0; a < 16; a++) {

      Wire1.beginTransmission(c);
      int result = 0;
      result = Wire1.endTransmission();
      if (result == 0) {
        
        //SerialAir_xiao.printf("%02x  ", c);
        SerialAir_xiao.print(c,HEX);
        SerialAir_xiao.print(" ");
      } else {
        SerialAir_xiao.print("--  ");
      }
      c++;
    }
    SerialAir_xiao.print("\nNEXT Wire");
  }

  delay(1000);
  
}
