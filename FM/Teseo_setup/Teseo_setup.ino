#include <Arduino.h>
#include <TinyGPSPlus.h>

SerialPIO Serial_GPS(14, 13, 4096);
TinyGPSPlus gps;



void setup() {
  Serial.begin(921600);

  Serial_GPS.begin(115200);

  // delay(500);

  // Serial_GPS.println("$PSTMSETPAR,1102,0xC*12"); // Baudrateを460800に変更
  // Serial_GPS.println("$PSTMSETPAR,1102,0xA*10"); // Baudrateを115200bpsに変更

  // sendCommand("PSTMSETPAR,1102,C"); // Baudrate 460800bpsに設定
  // Serial_GPS.println("$PSTMSETPAR,1102,0xD*15"); // Baudrateを921600に変更

  // delay(1000);
  // Serial_GPS.println("$PSTMSETPAR,1102,0x5*64"); // baudrateを9600bpsに変更
  
  // delay(1000);

  // Serial_GPS.println("$PSTMSETPAR,1303,0.20*06"); // 周期5Hz
  // Serial_GPS.println("$PSTMSETPAR,1303,0.10*05"); // 周期10Hz
  // Serial_GPS.println("$PSTMSETPAR,1303,1.0*35"); // 周期1Hz
  // Serial_GPS.println("$PSTMSETPAR,1303,0.25*03"); // 周期4Hz
  // Serial_GPS.println("$PSTMRESTOREPAR*11");

  // sendCommand("PSTMSETPAR,1303,0.10"); // 周期10Hz

  // delay(500);
  // Serial_GPS.println("$PSTMSAVEPAR*58");
  
  // delay(500);

  // Serial_GPS.println("$PSTMSRR*49");
  // delay(500); // 再起動待ち

  // Serial_GPS.begin(9600);

  pinMode(8, OUTPUT);
  pinMode(23, OUTPUT);

}

void loop() {
  // digitalWrite(8, HIGH);  // digitalWrite(23, HIGH);

  if (Serial_GPS.available() > 0) {

    gps.encode(Serial_GPS.read() );
    Serial.println(gps.location.lat());
    Serial.println(gps.time.second());

    // Serial.print((char)Serial_GPS.read());
  }

  // digitalWrite(8, LOW);
  // digitalWrite(23, LOW);

}