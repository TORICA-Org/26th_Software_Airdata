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
float calc_airspeed_ms(float sdp_differentialPressure_Pa, float dps_pressure_hPa, float dps_temperature_deg) {

  /*
  対気速度の計算
  計算式：\sqrt{| 2 \Delta P \times \frac{T}{P} \times \frac{R}{M} |}
  ただし R=8.314 \times 10^3 [J/(kmol \cdot K)], M=28.966 [kg/kmol] より R/M=287.026 [J/(kg \cdot K)] として計算
  */
  float airspeed_ms = sqrt(abs(2.0 * sdp_differentialPressure_Pa * ((dps_temperature_deg + 273.15) / (dps_pressure_hPa * 100.0) ) * 287.026 ) );

  return airspeed_ms;
}


//気圧高度の計算
float calc_pressureAltitude_m(float dps_pressure_hPa, float dps_temperature_deg) {
  //h = \frac{T + 273.15}{0.0065} \times ( (\frac{1013.25}{P})^{\frac{1}{5.526}} - 1 )
  float pressureAltitude_m = (dps_temperature_deg + 273.15) / 0.0065 * (pow( (1013.25 / dps_pressure_hPa), (1/5.526)) -1);
  return pressureAltitude_m;
}




