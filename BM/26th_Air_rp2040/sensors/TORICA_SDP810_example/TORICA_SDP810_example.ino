#include <Wire.h>
#include "TORICA_SDP810.h"

#ifndef SDP_I2C_ADDR
#define SDP_I2C_ADDR SDP8XX_I2C_ADDRESS_0
#endif

const unsigned long READ_INTERVAL_MS = 500; // how often to read (ms)

unsigned long lastRead = 0;

void setup() {
  Serial.begin(115200);
  while(!Serial) { delay(10); } // wait for Serial on some boards

  Wire.begin();

  Serial.println("Starting TORICA SDP810 example...");

  bool ok = TORICA_SDP810_init(Wire, SDP_I2C_ADDR);
  if(!ok){
    Serial.println("TORICA_SDP810: initialization failed");
  } else {
    Serial.println("TORICA_SDP810: initialized successfully");
  }
}

void loop() {
  unsigned long now = millis();
  if(now - lastRead >= READ_INTERVAL_MS){
    lastRead = now;

    float dp = TORICA_SDP810_getdifferentialPressure();

    Serial.print("Differential pressure: ");
    Serial.print(dp, 3);
    Serial.println(" Pa");
  }
}
