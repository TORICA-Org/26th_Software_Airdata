#include "Teseo_LIV3FL.h"

Teseo_LIV3FL gnss(Serial);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(5000);
  gnss.setBaudrate(115200);
  
  gnss.reboot()
}

void loop() {
  // put your main code here, to run repeatedly:
}
