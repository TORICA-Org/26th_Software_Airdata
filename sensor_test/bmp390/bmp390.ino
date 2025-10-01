// Designed specifically to work with the Adafruit BMP388 Breakout
//  ----> http://www.adafruit.com/products/3966
//  Written by Limor Fried & Kevin Townsend for Adafruit Industries.

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"

Adafruit_BMP3XX bmp;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Adafruit BMP388 / BMP390 test");
  Wire1.setSDA(26);
  Wire1.setSCL(27);
  Wire1.begin();

  if (!bmp.begin_I2C(0x77, &Wire1)) {   // hardware I2C mode, can pass in address & alt Wire
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);
}

void loop() {
  if (! bmp.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  float tempC = bmp.temperature;
  float press = bmp.pressure / 100.0;

  Serial.print("Temperature = ");
  Serial.print(tempC); Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(press); Serial.println(" hPa");

  Serial.println();
  delay(5000);
}