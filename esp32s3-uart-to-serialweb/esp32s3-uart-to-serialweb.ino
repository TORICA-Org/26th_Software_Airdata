#include <TORICA_SerialWeb.h>

void setup() {
  SerialWeb.begin("TORICA", "12345678");
  Serial1.begin(115200, SERIAL_8N1, 44, 43);
}

void loop() {
  if (Serial1.available() > 0) {
    SerialWeb.println(Serial1.readStringUntil('\n'));
  }
}
