//i2cデバイススキャナー．i2cが2系統備わっているマイコン向けに改造

#include <Wire.h> 

void scanI2CBus(TwoWire &wire, const char* busName) {
  byte error, address;
  int nDevices;

  Serial.print("Scanning ");
  Serial.print(busName);
  Serial.println("...");

  nDevices = 0;
  for (address = 1; address < 127; address++) {
    
    wire.beginTransmission(address);
    error = wire.endTransmission();

    if (error == 0) {
      Serial.print("  I2C device found on ");
      Serial.print(busName);
      Serial.print(" at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      nDevices++;
    } else if (error == 4) {
      Serial.print("  Unknown error on ");
      Serial.print(busName);
      Serial.print(" at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0) {
    Serial.print("  -> No I2C devices found on ");
    Serial.println(busName);
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("\nDual Port I2C Scanner");
  Serial.println("=======================");

  // Wire1 (I2C1)ポートの初期化
  Serial.println("Initializing Wire1 on SDA=26, SCL=27");
  Wire1.setSDA(26);
  Wire1.setSCL(27);
  Wire1.begin();

  // Wire (I2C0)ポートの初期化
  Serial.println("Initializing Wire on SDA=20, SCL=21");
  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire.begin();

  Serial.println();
}

void loop() {
  // 1つ目のI2Cバス (Wire) をスキャン
  scanI2CBus(Wire, "Wire (I2C0)");
  Serial.println("---");

  // 2つ目のI2Cバス (Wire1) をスキャン
  scanI2CBus(Wire1, "Wire1 (I2C1)");

  Serial.println("\nScan complete. Repeating in 5 seconds.");
  Serial.println("======================================\n");
  delay(5000); 
}
