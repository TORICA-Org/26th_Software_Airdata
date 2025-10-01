#include <Wire.h>
#include <AS5600.h>

AS5600 as5600_AOS(&Wire1);

volatile float AS5600_AOS_angle_deg = 0;

void setup() {
  while (!Serial);
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("AS5600_LIB_VERSION: ");
  Serial.println(AS5600_LIB_VERSION);
  Serial.println();

  Wire1.setSDA(26);
  Wire1.setSCL(27);
  Wire1.begin();
  int b = as5600_AOS.isConnected();
  if (b == 0) {
    Serial.print("Connect: ");
    Serial.println("AS5600_AOS setup error");
    delay(100);
  }
  if (b == 1) {
    Serial.print("Connect: ");
    Serial.println("AS5600_AOS setup OK");
    delay(100);
  }
}


void loop() {
  if (as5600_AOS.detectMagnet()) {
    float AS5600_AOS_rawAngle = as5600_AOS.rawAngle();
    AS5600_AOS_angle_deg = (AS5600_AOS_rawAngle * AS5600_RAW_TO_DEGREES);
  }
  Serial.println(AS5600_AOS_angle_deg);
}