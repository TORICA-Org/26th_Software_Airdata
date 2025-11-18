//各種関数の定義

#include <Arduino.h> // Arduinoの基本的な関数を使えるようにする
#include "TORICA_basicfunc.h" // このファイルで定義する宣言が書かれたヘッダーファイル(.h)をインクルード
#include "parameters.h" // 使用したい機能が書かれたヘッダーファイル(.h)をインクルード


//setup関数内で使うもの

void LED_init(void) {
  pinMode(LED_ICS, OUTPUT);
  pinMode(LED_Under, OUTPUT);
  pinMode(LED_Air_pico, OUTPUT);
  pinMode(LED_Air_xiao, OUTPUT);
}

//USBケーブルを差したときの起動猶予．処理を止めたいのでdelay関数使う
void init_delay_10sec(void) {
  //10秒間の猶予
  for (int i=1; i<=10; i++){
  digitalWrite(LED_ICS, HIGH);
  digitalWrite(LED_Under, HIGH);
  digitalWrite(LED_Air_pico, HIGH);
  digitalWrite(LED_Air_xiao, HIGH);
  delay(500);
  digitalWrite(LED_ICS, LOW);
  digitalWrite(LED_Under, LOW);
  digitalWrite(LED_Air_pico, LOW);
  digitalWrite(LED_Air_xiao, LOW);
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
  float airspeed_ms = sqrt(abs(2.0 * sdp_differentialPressure_Pa * ((bmp_temperature_deg + 273.15) / (bmp_pressure_hPa * 100.0) ) * 287.026 ) );

  return airspeed_ms;
}


//気圧高度の計算
float calc_pressureAltitude_m(float bmp_pressure_hPa, float bmp_temperature_deg) {
  //h = \frac{T + 273.15}{0.0065} \times ( (\frac{1013.25}{P})^{\frac{1}{5.526}} - 1 )
  float pressureAltitude_m = (bmp_temperature_deg + 273.15) / 0.0065 * (pow( (1013.25 / bmp_pressure_hPa), (1/5.526)) -1);
  return pressureAltitude_m;
}


/*---------------------------------------

//UART いったん雑に実装
void polling_UART(void) {
  //ICS
  data_ics_angle = ics.read_angle();
  if (data_ics_angle > 0) {
    digitalWrite(LED_ICS, !digitalRead(LED_ICS));
  }

  //Under読み取り
  static unsigned long int last_under_time_ms = 0;
  int readnum = Under_UART.readUART();
  if (readnum == under_data_num) {
    last_under_time_ms = millis();
    digitalWrite(L_Under, !digitalRead(L_Under));
    data_under_bmp_pressure_hPa = Under_UART.UART_data[0];
    data_under_bmp_temperature_deg = Under_UART.UART_data[1];
    data_under_bmp_altitude_m = Under_UART.UART_data[2];
    data_under_bmp_altitude_m = Under_UART.UART_data[3];
    filtered_under_bmp_altitude_m.add(data_under_bmp_altitude_m);
    if (flight_phase == PLATFORM) {
      under_bmp_altitude_platform_m.add(data_under_bmp_altitude_m);
    }
    filtered_under_urm_altitude_m.add(data_under_urm_altitude_m);
  }
  if (millis() - last_under_time_ms > 1000) {
    // 超音波高度のみ冗長系がないため，データが来なければ8mとして高度推定に渡す．
    // 測定範囲外のときは10mになり，9m以上でテイクオフ判断をするため故障時は8m
    // filtered_under_urm_altitude_m.add(8.0);
    // ToDo 明示的にis_aliveを作るべき．値の処理によって7変わる．
    under_is_alive = false;
  } else {
    under_is_alive = true;
  }
}



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
