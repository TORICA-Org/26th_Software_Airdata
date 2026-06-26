#include "Teseo_LIV3FL.h"

Teseo_LIV3FL gnss(Serial1);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, D7, D6);
  // delay(1000);
  // gnss.setBaudrate(460800);
  // delay(10);
  // gnss.save();
  // delay(10);
  // gnss.reboot();
  // delay(10);

  // Serial1.begin(460800, SERIAL_8N1, D7, D6);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial1.available()) {
    Serial.write(Serial1.read());
  }
}
