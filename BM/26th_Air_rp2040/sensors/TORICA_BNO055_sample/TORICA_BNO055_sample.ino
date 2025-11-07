#include <Arduino.h>
#include <Wire.h>
#include "TORICA_BNO055.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("TORICA BNO055 sample start");

  Wire.begin();

  // BNO055初期化（アドレスがデフォルト0x28の場合）
  TORICA_BNO055_init(Wire, 0x28);

  // 小休止
  delay(100);
}

void loop() {
  float roll = 0.0f;
  float pitch = 0.0f;
  float yaw = 0.0f;

  // TORICA_BNO055_getData が roll/pitch/yaw (degrees) を返す
  TORICA_BNO055_getData(roll, pitch, yaw);

  // 出力
  Serial.print("Roll: ");
  Serial.print(roll, 2);
  Serial.print(" deg, Pitch: ");
  Serial.print(pitch, 2);
  Serial.print(" deg, Yaw: ");
  Serial.print(yaw, 2);
  Serial.println(" deg");

  delay(200);
}