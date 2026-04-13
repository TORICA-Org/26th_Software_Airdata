#define DEBUG_MODE

#include <SerialWeb.h>
#include "parameters.h"
#include <TORICA_UART.h>


constexpr char SSID[] = "SerialWeb";
constexpr char PASSWORD[] = "12345678";


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  // 内蔵LEDを出力モードに設定
  Serial.begin(460800);
  SerialWeb.begin(SSID, PASSWORD);  // Serialなどと同様に初期化します．
  Serial1.begin(460800, SERIAL_8N1, D7, D6);
  delay(1000);
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);  // 内蔵LED ON
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);  // 内蔵LED OFF
    delay(1000);
  }
}

void loop() {

  // ログを受信＆受信データを変数と紐づけ
  receiveLog();


  // send関数で送信したデータは，ダッシュボードに表示されます．
  //sprintf(value, "%ld", millis());
  //SerialWeb.send(label, value);

    //char label[] = "NOW_TIME";
  //char value[32];
  char label1[] = "time_ms";
  char value1[32];
  sprintf(value1, "%d", time_ms);
  SerialWeb.send(label1, value1);

  char label2[] = "air_bno_accx_mss";
  char value2[32];
  sprintf(value2, "%.2f", data_psd_bno_accx_mss);
  SerialWeb.send(label2, value2);

  char label3[] = "air_bno_accy_mss";
  char value3[32];
  sprintf(value3, "%.2f", data_psd_bno_accy_mss);
  SerialWeb.send(label3, value3);

  char label4[] = "air_bno_accz_mss";
  char value4[32];
  sprintf(value4, "%.2f", data_psd_bno_accz_mss);
  SerialWeb.send(label4, value4);

  char label5[] = "air_bno_qw";
  char value5[32];
  sprintf(value5, "%.2f", data_psd_bno_qw);
  SerialWeb.send(label5, value5);

  char label6[] = "air_bno_qx";
  char value6[32];
  sprintf(value6, "%.2f", data_psd_bno_qx);
  SerialWeb.send(label6, value6);

  char label7[] = "air_bno_qy";
  char value7[32];
  sprintf(value7, "%.2f", data_psd_bno_qy);
  SerialWeb.send(label7, value7);

  char label8[] = "air_bno_qz";
  char value8[32];
  sprintf(value8, "%.2f", data_psd_bno_qz);
  SerialWeb.send(label8, value8);

  char label9[] = "air_bno_roll";
  char value9[32];
  sprintf(value9, "%.2f", data_psd_bno_roll);
  SerialWeb.send(label9, value9);

  char label10[] = "air_bno_pitch";
  char value10[32];
  sprintf(value10, "%.2f", data_psd_bno_pitch);
  SerialWeb.send(label10, value10);

  char label11[] = "air_bno_yaw";
  char value11[32];
  sprintf(value11, "%.2f", data_psd_bno_yaw);
  SerialWeb.send(label11, value11);

  char label12[] = "air_bmp_pressure_hPa";
  char value12[32];
  sprintf(value12, "%.2f", data_air_bmp_pressure_hPa);
  SerialWeb.send(label12, value12);

  char label13[] = "air_bmp_temperature_deg";
  char value13[32];
  sprintf(value13, "%.2f", data_air_bmp_temperature_deg);
  SerialWeb.send(label13, value13);

  char label14[] = "air_bmp_altitude_m";
  char value14[32];
  sprintf(value14, "%.2f", data_air_bmp_altitude_m);
  SerialWeb.send(label14, value14);

  char label15[] = "air_sdp_differentialPressure_Pa";
  char value15[32];
  sprintf(value15, "%.2f", data_air_sdp_differentialPressure_Pa);
  SerialWeb.send(label15, value15);
  
  char label16[] = "air_sdp_airspeed_ms";
  char value16[32];
  sprintf(value16, "%.2f", data_air_sdp_airspeed_ms);
  SerialWeb.send(label16, value16);

  char label17[] = "air_AoA_angle_deg";
  char value17[32];
  sprintf(value17, "%.2f", data_air_AoA_angle_deg);
  SerialWeb.send(label17, value17);

  char label18[] = "air_AoS_angle_deg";
  char value18[32];
  sprintf(value18, "%.2f", data_air_AoS_angle_deg);
  SerialWeb.send(label18, value18);

  char label19[] = "air_gps_hour";
  char value19[32];
  sprintf(value19, "%u", data_air_gps_hour);
  SerialWeb.send(label19, value19);

  char label20[] = "air_gps_minute";
  char value20[32];
  sprintf(value20, "%u", data_air_gps_minute);
  SerialWeb.send(label20, value20);

  char label21[] = "air_gps_second";
  char value21[32];
  sprintf(value21, "%u", data_air_gps_second);
  SerialWeb.send(label21, value21);

  char label22[] = "air_gps_centisecond";
  char value22[32];
  sprintf(value22, "%u", data_air_gps_centisecond);
  SerialWeb.send(label22, value22);

  char label23[] = "air_gps_latitude_deg";
  char value23[32];
  sprintf(value23, "%10.7f", data_air_gps_latitude_deg);
  SerialWeb.send(label23, value23);

  char label24[] = "air_gps_longitude_deg";
  char value24[32];
  sprintf(value24, "%10.7f", data_air_gps_longitude_deg);
  SerialWeb.send(label24, value24);

  char label25[] = "air_gps_altitude_m";
  char value25[32];
  sprintf(value25, "%5.2f", data_air_gps_altitude_m);
  SerialWeb.send(label25, value25);

  char label26[] = "air_gps_groundspeed_ms";
  char value26[32];
  sprintf(value26, "%5.2f", data_air_gps_groundspeed_ms);
  SerialWeb.send(label26, value26);

  char label27[] = "ics_angle";
  char value27[32];
  sprintf(value27, "%d", data_ics_angle);
  SerialWeb.send(label27, value27);

  char label28[] = "under_bmp_pressure_hPa";
  char value28[32];
  sprintf(value28, "%.2f", data_under_bmp_pressure_hPa);
  SerialWeb.send(label28, value28);

  char label29[] = "under_bmp_temperature_deg";
  char value29[32];
  sprintf(value29, "%.2f", data_under_bmp_temperature_deg);
  SerialWeb.send(label29, value29);

  char label30[] = "under_bmp_altitude_m";
  char value30[32];
  sprintf(value30, "%.2f", data_under_bmp_altitude_m);
  SerialWeb.send(label30, value30);

  char label31[] = "under_urm_altitude_m";
  char value31[32];
  sprintf(value31, "%.2f", data_under_urm_altitude_m);
  SerialWeb.send(label31, value31);

  char label32[] = "under_tsd20_altitude_m";
  char value32[32];
  sprintf(value32, "%.2f", data_under_tsd20_altitude_m);
  SerialWeb.send(label32, value32);

  char label33[] = "estimated_altitude_lake_m";
  char value33[32];
  sprintf(value33, "%.2f", estimated_altitude_lake_m);
  SerialWeb.send(label33, value33);

  char label34[] = "altitude_bmp_urm_offset_m";
  char value34[32];
  sprintf(value34, "%.2f", data_altitude_bmp_urm_offset_m);
  SerialWeb.send(label34, value34);

  char label35[] = "flight_phase";
  char value35[32];
  sprintf(value35, "%d", flight_phase);
  SerialWeb.send(label35, value35);

  char label36[] = "speed_level";
  char value36[32];
  sprintf(value36, "%d", speed_level);
  SerialWeb.send(label36, value36);




  // print関数やprintln関数はSerialなどと同様に使用できます．
  // SerialWeb.print("Hello, ");
  // SerialWeb.println("world.");

  // 読み取りは，readString関数のみの実装です．
  // available関数はbool型です．
  if (SerialWeb.available()) {
    // メモリの上書き問題が解決できていません．
    // 半角英数字13文字までなら正常動作します．
    String msg = SerialWeb.readString();
    Serial.printf("readString: %s\n", msg);
  }

  Serial.print("AoS_angle_deg:");
  Serial.print(data_air_AoS_angle_deg);
  Serial.print("\t");
  Serial.print("AoA_angle_deg:");
  Serial.print(data_air_AoA_angle_deg);
  Serial.print("\t");
  Serial.print("Roll:");
  Serial.print(data_psd_bno_roll);
  Serial.print("\t");
  Serial.print("Pitch:");
  Serial.print(data_psd_bno_pitch);
  Serial.print("\t");
  Serial.print("Yaw:");
  Serial.print(data_psd_bno_yaw);
  Serial.print("\t");
  Serial.print("BMP_Temp:");
  Serial.print(data_air_bmp_temperature_deg);
  Serial.print("\t");
  Serial.print("BMP_Pres:");
  Serial.print(data_air_bmp_pressure_hPa);
  Serial.print("\t");
  Serial.print("BMP_Alt:");
  Serial.print(data_air_bmp_altitude_m);
  Serial.print("\t");
  Serial.print("Airspeed:");
  Serial.print(data_air_sdp_airspeed_ms);
  Serial.println();

  delay(100);
}