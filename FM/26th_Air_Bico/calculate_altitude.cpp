/*---------------------------

ファイルの役割：高度計算
最終更新日：2026/02/17 17:18
更新内容：ファイル作成

----------------------------*/


/* 基本動作 */
/*
・気圧高度を計算する．
・得られた超音波，LiDAR，気圧高度をもとに，以下のプロセスでパイロットに伝える高度を決定する．

  1. 8m以上→気圧高度のみ．(urm_is_reliable = false)
  2. 8m未満→超音波高度に切り替え．(urm_is_reliable = true)

*/

#pragma once
#include <Arduino.h>
#include "parameters.h"
#include <TORICA_MoveAve.h>

const float const_platform_altitude_m = 10.6f;  // プラットフォームの高度[m]


// 高度
TORICA_MoveAve<5> filtered_under_bmp_altitude_m(0);   // 直近5回で取得した機体下電装における気圧高度の平均
TORICA_MoveAve<5> filtered_air_bmp_altitude_m(0);     // 直近5回で取得したエアデータ電装における気圧高度の平均
TORICA_MoveAve<5> filtered_fslg_bmp_altitude_m(0);     // 直近5回で取得した胴体桁電装における気圧高度の平均

TORICA_MoveAve<50> air_bmp_altitude_platform_m(0);    // プラホ上で直近50回で取得したエアデータ電装における気圧高度の平均
TORICA_MoveAve<50> under_bmp_altitude_platform_m(0);  // プラホ上で直近50回で取得した機体下電装における気圧高度の平均
TORICA_MoveAve<50> fslg_bmp_altitude_platform_m(0);    // プラホ上で直近50回で取得した胴体桁電装における気圧高度の平均


#include <QuickStats.h>
float bmp_altitude_lake_array_m[3]; // Air, Under, fslgの気圧高度を格納
QuickStats bmp_altitude_lake_m; // Air, Under, fslgの気圧高度の中央値をとるため

// 超音波高度
TORICA_MoveAve<3> filtered_under_urm_altitude_m(0); // 直近3回で取得した超音波高度の平均

#include <TORICA_MoveMedian.h>
TORICA_MoveMedian<400> altitude_bmp_urm_offset_m(0); // 直近400回(=100Hzで測定した4秒分のデータ)の気圧高度と超音波高度の差の中央値


// この関数を実行する前に，すべてのセンサー値を取得しておくこと
void calculate_altitude() {

  // 気圧高度計算
  data_air_bmp_altitude_m = (powf(1013.25 / data_air_bmp_pressure_hPa, 1 / 5.257) - 1) * (data_air_bmp_temperature_deg + 273.15) / 0.0065;
  filtered_air_bmp_altitude_m.add(data_air_bmp_altitude_m);

  if (takeoff == false) { // 離陸前はプラットフォーム上の平均高度を更新する
    air_bmp_altitude_platform_m.add(data_air_bmp_altitude_m);
    under_bmp_altitude_platform_m.add(data_under_bmp_altitude_m);
    fslg_bmp_altitude_platform_m.add(data_fslg_bmp_altitude_m);
  }

  // (現在の高度) - (プラットフォーム上の平均高度) + (プラホの高度)
  bmp_altitude_lake_array_m[0] = filtered_air_bmp_altitude_m.get() - air_bmp_altitude_platform_m.get() + const_platform_altitude_m;
  bmp_altitude_lake_array_m[1] = filtered_under_bmp_altitude_m.get() - under_bmp_altitude_platform_m.get() + const_platform_altitude_m;
  bmp_altitude_lake_array_m[2] = filtered_fslg_bmp_altitude_m.get() - fslg_bmp_altitude_platform_m.get() + const_platform_altitude_m;

  filtered_bmp_altitude_m = bmp_altitude_lake_m.median(bmp_altitude_lake_array_m, 3); // 3つの気圧高度の中央値をとる
  /* 気圧高度計算ここまで */

  /* 超音波高度フィルタリング */
  filtered_under_urm_altitude_m.add(data_under_urm_altitude_m);

  // 超音波高度が信頼できるか判別
  if (data_under_urm_altitude_m > 8.0) {
    urm_is_reliable = false;
  } else {
    urm_is_reliable = true;
    filtered_urm_altitude_m = filtered_under_urm_altitude_m.get();
  }

}