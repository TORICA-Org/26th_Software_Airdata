//各種関数の定義

#include <Arduino.h>  // Arduinoの基本的な関数を使えるようにする
#include "TORICA_basicfunc.h"
#include "TORICA_parameters.h"

//TORICA_ICS / TORICA_UART のメソッドを使うため
#include <TORICA_ICS.h>
#include <TORICA_UART.h>


//setup関数内で使うもの

void LED_init(void) {
  pinMode(LED_ICS, OUTPUT);
  pinMode(LED_Under, OUTPUT);
  pinMode(LED_Air_pico, OUTPUT);
  pinMode(LED_Air_xiao, OUTPUT);
  pinMode(LED_GPS, OUTPUT);
  pinMode(LED_SD, OUTPUT);
}

//USBケーブルを差したときの起動猶予．処理を止めたいのでdelay関数使う
void init_delay_10sec(void) {
  //10秒間の猶予
  for (int i = 1; i <= 10; i++) {
    digitalWrite(LED_ICS, HIGH);
    digitalWrite(LED_Under, HIGH);
    digitalWrite(LED_Air_pico, HIGH);
    digitalWrite(LED_Air_xiao, HIGH);
    digitalWrite(LED_GPS, HIGH);
    digitalWrite(LED_SD, HIGH);
    delay(500);
    digitalWrite(LED_ICS, LOW);
    digitalWrite(LED_Under, LOW);
    digitalWrite(LED_Air_pico, LOW);
    digitalWrite(LED_Air_xiao, LOW);
    digitalWrite(LED_GPS, LOW);
    digitalWrite(LED_SD, LOW);
    delay(500);
  }
}


//対気速度の計算
float calc_airspeed_ms(float sdp_differentialPressure_Pa, float bmp_pressure_hPa, float bmp_temperature_deg) {

  /*
  対気速度の計算
  計算式：\sqrt{| 2 \Delta P \times \frac{T}{P} \times \frac{R}{M} |}
  ただし R=8.314 \times 10^3 [J/(kmol \cdot K)], M=28.966 [kg/kmol] より R/M=287.026 [J/(kg \cdot K)] として計算
  */
  float airspeed_ms = sqrt(abs(2.0 * sdp_differentialPressure_Pa * ((bmp_temperature_deg + 273.15) / (bmp_pressure_hPa * 100.0)) * 287.026));

  return airspeed_ms;
}


//気圧高度の計算
float calc_pressureAltitude_m(float bmp_pressure_hPa, float bmp_temperature_deg) {
  //h = \frac{T + 273.15}{0.0065} \times ( (\frac{1013.25}{P})^{\frac{1}{5.526}} - 1 )
  float pressureAltitude_m = (bmp_temperature_deg + 273.15) / 0.0065 * (pow((1013.25 / bmp_pressure_hPa), (1 / 5.526)) - 1);
  return pressureAltitude_m;
}


//UART送信関連
bool SD_setting(Stream &serialPort /* シリアルポート名を入力 */, const char *option /* マイコン名・場所を記述 */) {
  static int loop_count = 0;
  if (loop_count == 0) {
    sprintf(UART_SD, "time_ms,data_air_bno_accx_mss,data_air_bno_accy_mss,data_air_bno_accz_mss,data_air_bno_qw,data_air_bno_qx,data_air_bno_qy,data_air_bno_qz,data_air_bno_roll, data_air_bno_pitch,");  //10個
  } else if (loop_count == 1) {
    sprintf(UART_SD, "data_air_bno_yaw,data_air_bmp_pressure_hPa,data_air_bmp_temperature_deg,data_air_bmp_altitude_m,data_air_sdp_differentialPressure_Pa,data_air_sdp_airspeed_ms,data_air_AoA_angle_deg,data_air_AoS_angle_deg,data_air_gps_hour,data_air_gps_minute,");  //10個
  } else if (loop_count == 2) {
    sprintf(UART_SD, "data_air_gps_second,data_air_gps_centisecond,data_air_gps_latitude_deg,data_air_gps_longitude_deg,data_air_gps_altitude_m,data_air_gps_groundspeed_ms,data_ics_angle,data_under_bmp_pressure_hPa,data_under_bmp_temperature_deg,data_under_bmp_altitude_m,");  //10個
  } else {
    sprintf(UART_SD, "data_under_urm_altitude_m,data_under_tsd20_altitude_m,estimated_altitude_lake_m,altitude_bmp_urm_offset_m.get(),flight_phase,speed_level");  //6個
  }
  loop_count++;
  serialPort.flush();
  serialPort.print(UART_SD);
  return true;
}



//旧send_SD()
bool send_data(Stream &serialPort /* 送信用UARTポートを入力 */, const char *option /* 実行マイコン名・場所を記述 */) {
  //Air_Bico -> Air_xiao用
  if (strcmp(option, "air_xiao") == 0) {
    time_ms = millis();
    static int loop_count = 0;
    static uint32_t SD_last_send_time = 0;

    if (loop_count == 0) {
      sprintf(UART_SD, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,", time_ms, data_air_bno_accx_mss, data_air_bno_accy_mss, data_air_bno_accz_mss, data_air_bno_qw, data_air_bno_qx, data_air_bno_qy, data_air_bno_qz, data_air_bno_roll, data_air_bno_pitch);  //10個
    } else if (loop_count == 1) {
      sprintf(UART_SD, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%u,%u,", data_air_bno_yaw, data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg, data_air_bmp_altitude_m, data_air_sdp_differentialPressure_Pa, data_air_sdp_airspeed_ms, data_air_AoA_angle_deg, data_air_AoS_angle_deg, data_air_gps_hour, data_air_gps_minute);  //10個
    } else if (loop_count == 2) {
      sprintf(UART_SD, "%u,%u,%10.7lf,%10.7f,%5.2f,%5.2f,%d,%.2f,%.2f,%.2f,", data_air_gps_second, data_air_gps_centisecond, data_air_gps_latitude_deg, data_air_gps_longitude_deg, data_air_gps_altitude_m, data_air_gps_groundspeed_ms, data_ics_angle, data_under_bmp_pressure_hPa, data_under_bmp_temperature_deg, data_under_bmp_altitude_m);  //10個
    } else {
      sprintf(UART_SD, "%.2f,%.2f,%.2f,%.2f,%d,%d\n", data_under_urm_altitude_m, data_under_tsd20_altitude_m, estimated_altitude_lake_m, altitude_bmp_urm_offset_m.get(), flight_phase, speed_level);  //6個
      loop_count = -1;
    }
    SD_last_send_time = millis();
    loop_count++;
    //バッファをクリアして新しいデータを書き込む
    serialPort.flush();
    serialPort.print(UART_SD);
    return true;

    //Under用
  } else if (strcmp(option, "under") == 0) {
    sprintf(sendUART_BUF, "%.2f,%.2f,%.2f,%.2f,%.3f\n", data_under_bmp_pressure_hPa, data_under_bmp_temperature_deg, data_under_bmp_altitude_m, data_under_urm_altitude_m, data_under_tsd20_altitude_m);
    serialPort.print(sendUART_BUF);

    return true;
  } else {

//未定義のモードが指定された場合
#ifdef DEBUG_MODE
    Serial.println("send_data(): Invalid option");
#endif
    return false;
  }
}




//polling_UART() Bicoとテレメトリ用xiao向け
bool polling_UART(const char *option, TORICA_ICS *ICS, TORICA_UART *Under, TORICA_UART *air_rp2040_uart) {

  //Air_Bico向け．Under -> BicoとICS -> Bico
  if (strcmp(option, "air_rp2040") == 0) {

    //第2，3引数に引数が指定されているか確認
    if (ICS != nullptr && Under != nullptr) {

      //ここからICSとUnderの読み取り処理
      //ICS読み取り
      data_ics_angle = ICS->read_Angle();
      if (data_ics_angle > 0) {
        digitalWrite(LED_ICS, !digitalRead(LED_ICS));
      }
      //Under読み取り
      static unsigned long int last_under_time_ms = 0;
      int readnum = Under->readUART();
      int under_data_num = 5;  //受信データ数
      if (readnum == under_data_num) {
        last_under_time_ms = millis();
        digitalWrite(LED_Under, !digitalRead(LED_Under));
        data_under_bmp_pressure_hPa = Under->UART_data[0];
        data_under_bmp_temperature_deg = Under->UART_data[1];
        data_under_bmp_altitude_m = Under->UART_data[2];
        data_under_urm_altitude_m = Under->UART_data[3];
        data_under_tsd20_altitude_m = Under->UART_data[4];

        //気圧高度のフィルタリング方法を要検討
        //filtered_under_bmp_altitude_m.add(data_under_bmp_altitude_m);
        if (flight_phase == PLATFORM) {
          //under_bmp_altitude_platform_m.add(data_under_bmp_altitude_m);
        }
        filtered_under_urm_altitude_m.add(data_under_urm_altitude_m);
        return true;
      }
      if (millis() - last_under_time_ms > 1000) {
        // 超音波高度のみ冗長系がないため，データが来なければ8mとして高度推定に渡す．
        // 測定範囲外のときは10mになり，9m以上でテイクオフ判断をするため故障時は8m
        // filtered_under_urm_altitude_m.add(8.0);
        // ToDo 明示的にis_aliveを作るべき．値の処理によって7変わる．
        under_is_alive = false;
        return false;
      } else {
        under_is_alive = true;
        return true;
      }
    } else {
#ifdef DEBUG_MODE
      Serial.println("polling_UART(): ICS or Under data not provided");
#endif
      return false;
    }

    //Air_xiao向け．Bico -> Air_xiao
  } else if (strcmp(option, "air_xiao") == 0) {
    if (air_rp2040_uart != nullptr) {
      //ここからAir_xiao向けの受信処理
      //Air_Bico読み取り
      static unsigned long int last_bico_time_ms = 0;
      int readnum = air_rp2040_uart->readUART();
      int bico_data_num = 36;  //受信データ数
      if (readnum == bico_data_num) {
        last_bico_time_ms = millis();
        //ここでLED点滅させる？

        //データ格納
        time_ms = air_rp2040_uart->UART_data[0];
        data_air_bno_accx_mss = air_rp2040_uart->UART_data[1];
        data_air_bno_accy_mss = air_rp2040_uart->UART_data[2];
        data_air_bno_accz_mss = air_rp2040_uart->UART_data[3];
        data_air_bno_qw = air_rp2040_uart->UART_data[4];
        data_air_bno_qx = air_rp2040_uart->UART_data[5];
        data_air_bno_qy = air_rp2040_uart->UART_data[6];
        data_air_bno_qz = air_rp2040_uart->UART_data[7];
        data_air_bno_roll = air_rp2040_uart->UART_data[8];
        data_air_bno_pitch = air_rp2040_uart->UART_data[9];
        data_air_bno_yaw = air_rp2040_uart->UART_data[10];
        data_air_bmp_pressure_hPa = air_rp2040_uart->UART_data[11];
        data_air_bmp_temperature_deg = air_rp2040_uart->UART_data[12];
        data_air_bmp_altitude_m = air_rp2040_uart->UART_data[13];
        data_air_sdp_differentialPressure_Pa = air_rp2040_uart->UART_data[14];
        data_air_sdp_airspeed_ms = air_rp2040_uart->UART_data[15];
        data_air_AoA_angle_deg = air_rp2040_uart->UART_data[16];
        data_air_AoS_angle_deg = air_rp2040_uart->UART_data[17];
        data_air_gps_hour = (uint8_t)air_rp2040_uart->UART_data[18];
        data_air_gps_minute = (uint8_t)air_rp2040_uart->UART_data[19];
        data_air_gps_second = (uint8_t)air_rp2040_uart->UART_data[20];
        data_air_gps_centisecond = (uint8_t)air_rp2040_uart->UART_data[21];
        data_air_gps_latitude_deg = air_rp2040_uart->UART_data[22];
        data_air_gps_longitude_deg = air_rp2040_uart->UART_data[23];
        data_air_gps_altitude_m = air_rp2040_uart->UART_data[24];
        data_air_gps_groundspeed_ms = air_rp2040_uart->UART_data[25];
        data_ics_angle = (int)air_rp2040_uart->UART_data[26];
        data_under_bmp_pressure_hPa = air_rp2040_uart->UART_data[27];
        data_under_bmp_temperature_deg = air_rp2040_uart->UART_data[28];
        data_under_bmp_altitude_m = air_rp2040_uart->UART_data[29];
        data_under_urm_altitude_m = air_rp2040_uart->UART_data[30];
        data_under_tsd20_altitude_m = air_rp2040_uart->UART_data[31];
        estimated_altitude_lake_m = air_rp2040_uart->UART_data[32];
        data_altitude_bmp_urm_offset_m = air_rp2040_uart->UART_data[33];
        flight_phase = (FlightPhase)(int)air_rp2040_uart->UART_data[34];
        speed_level = (SpeedLevel)(int)air_rp2040_uart->UART_data[35];

        return true;
      }

      //bicoとの通信が途絶えた場合
      if (millis() - last_bico_time_ms > 1000) {

        air_bico_is_alive = false;
        return false;
      } else {
        air_bico_is_alive = true;
        return true;
      }
      return true;

      //未定義のモードが指定された場合
    } else {
      
      #ifdef DEBUG_MODE
      Serial.println("polling_UART(): Invalid option");
      #endif

      return false;
    }
  }

  //どの場合にも該当しなかった場合
  return false;
}




  /*------------------------------


//要検討！！！！
//flight phase決定

void determine_flight_phase() {
  //発進判定のため，IMU測定はここで行う
  read_main_bno();
  //Serial.print("距離:");
  //Serial.println(filtered_under_urm_altitude_m.get());
  static unsigned long int takeoff_time_ms = 0;
  switch (flight_phase) {
    case PLATFORM:
      {
        static int over_urm_range_count = 0;
        if (filtered_under_urm_altitude_m.get() > 6.0) {
          over_urm_range_count++;
        } else {
          over_urm_range_count = 0;
        }
        bool over_urm_range = false;
        // 超音波が測定不能な状態が2秒以上続いたとき
        if (over_urm_range_count >= 100) {
          over_urm_range = true;
        }
        // 気圧センサにより下降したと判断したとき
        bool descending = estimated_altitude_lake_m < 10.2;
        // x軸方向の加速度またはy軸方向の急激な加速と超音波センサと気圧センサによる下降判断そしてマイコンが起動してから10秒後
        if ((over_urm_range || descending) && millis() > 15000){
          TAKEOFF = true;
          flight_phase = HIGH_LEVEL;
        }
        if (over_urm_range && millis() > 15000) {
          SerialWireless.print("\n\nover_urm_range\n\n");
        }
        if (descending && millis() > 15000) {
          SerialWireless.print("\n\ndescending\n\n");
        }
      }
      break;
    case HIGH_LEVEL:
      if ( 0.3 <= filtered_under_urm_altitude_m.get() && filtered_under_urm_altitude_m.get() <= 1.5){
        flight_phase = MID_LEVEL;
      }
      else if ( filtered_under_urm_altitude_m.get() < 0.3){
        flight_phase = LOW_LEVEL;
      }
      else{
        flight_phase = HIGH_LEVEL;
      }
      break;
    case MID_LEVEL:
      if ( 0.3 <= filtered_under_urm_altitude_m.get() && filtered_under_urm_altitude_m.get() <= 1.5){
        flight_phase = MID_LEVEL;
      }
      else if ( filtered_under_urm_altitude_m.get() < 0.3){
        flight_phase = LOW_LEVEL;
      }
      else{
        flight_phase = HIGH_LEVEL;
      }
      break;
    case LOW_LEVEL:
      if ( 0.3 <= filtered_under_urm_altitude_m.get() && filtered_under_urm_altitude_m.get() <= 1.5){
        flight_phase = MID_LEVEL;
      }
      else if ( filtered_under_urm_altitude_m.get() < 0.3){
        flight_phase = LOW_LEVEL;
      }
      else{
        flight_phase = HIGH_LEVEL;
      }
      break;
    default:
      break;
  }
  //速度レベル判断
  if (filtered_airspeed_ms.get() > 10.8) {
    speed_level = FAST;
  }
  else if (filtered_airspeed_ms.get() > 9.5) {

    speed_level = NORMAL;
  }
  else {
    speed_level = SLOW;
  }
}


-------------------------------*/
