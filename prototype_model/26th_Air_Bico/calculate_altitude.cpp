/*---------------------------

ファイルの役割：高度計算
最終更新日：2026/02/17 17:18
更新内容：ファイル作成

----------------------------*/

#pragma once
#include <Arduino.h>
#include "parameters.h"

//移動平均の計算
#include <TORICA_MoveAve.h>
// 対気速度
TORICA_MoveAve<5> filtered_airspeed_ms(0);  // 直近5回で取得した機速の平均
//高度
TORICA_MoveAve<5> filtered_under_bmp_altitude_m(0);   // 直近5回で取得した機体下電装における気圧高度の平均
TORICA_MoveAve<5> filtered_air_bmp_altitude_m(0);     // 直近5回で取得したエアデータ電装における気圧高度の平均
TORICA_MoveAve<50> under_bmp_altitude_platform_m(0);  // 直近50回で取得した機体下電装における気圧高度の平均
TORICA_MoveAve<50> air_bmp_altitude_platform_m(0);    // 直近50回で取得したエアデータ電装における気圧高度の平均

#include <QuickStats.h>
float bmp_altitude_lake_array_m[3];
QuickStats bmp_altitude_lake_m;

// 超音波高度(対地高度)
TORICA_MoveAve<3> filtered_under_urm_altitude_m(0);

#include <TORICA_MoveMedian.h>
TORICA_MoveMedian<400> altitude_bmp_urm_offset_m(0);



//この関数を実行する前に，read_bmp_air()を実行すること
void calculate_altitude() {

  data_air_bmp_altitude_m = (powf(1013.25 / data_air_bmp_pressure_hPa, 1 / 5.257) - 1) * (data_air_bmp_temperature_deg + 273.15) / 0.0065;
  filtered_air_bmp_altitude_m.add(data_air_bmp_altitude_m);

  if (flight_phase == PLATFORM) {
    air_bmp_altitude_platform_m.add(data_air_bmp_altitude_m);
  }

  bmp_altitude_lake_array_m[0] = filtered_air_bmp_altitude_m.get() - air_bmp_altitude_platform_m.get() + const_platform_altitude_m;
  bmp_altitude_lake_array_m[1] = filtered_under_bmp_altitude_m.get() - under_bmp_altitude_platform_m.get() + const_platform_altitude_m;

  // estimated_altitude_lake_m = (bmp_altitude_lake_array_m[0] - altitude_bmp_urm_offset_m + bmp_altitude_lake_array_m[1]) / 2;

  float a = bmp_altitude_lake_array_m[0];
  float b = bmp_altitude_lake_array_m[1];
  estimated_altitude_lake_m = (a - altitude_bmp_urm_offset_m.get() + b) / 2.0f;

  static int transtion_count = 0;
  if (flight_phase == MID_LEVEL || flight_phase == LOW_LEVEL) {
    // 気圧センサが本来より低い値ならオフセットは正
    altitude_bmp_urm_offset_m.add(filtered_under_urm_altitude_m.get() - estimated_altitude_lake_m);

    if (transtion_count < 500) {
      transtion_count++;
    }
    float ratio = 1;
    if (transtion_count < 500) {
      ratio = 0;
    }
    if (transtion_count > 200) {
      ratio = (float)(transtion_count - 200) / 300.0;
    }
    // 気圧センサが本来より低い値なら正のオフセットを足す
    estimated_altitude_lake_m += altitude_bmp_urm_offset_m.get() * ratio;
  }
}