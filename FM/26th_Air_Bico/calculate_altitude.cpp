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

TORICA_MoveAve<5> filtered_tsd20_altitude_m(0);
// 機体下電装のTSD20 LiDAR高度平均

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


// 物理的な気圧高度計算式を関数化（何をしているかをわかりやすくする）
float calculate_bmp_altitude(float pressure_hPa, float temperature_deg) {
    if (pressure_hPa <= 0.0f) return 0.0f;
    return (powf(1013.25f / pressure_hPa, 1.0f / 5.257f) - 1.0f) * (temperature_deg + 273.15f) / 0.0065f;
}

// この関数を実行する前に，すべてのセンサー値を取得しておくこと
void calculate_altitude(float air_press_hPa, float air_temp_deg, float under_press_hPa, float under_temp_deg, float fslg_press_hPa, float fslg_temp_deg, float under_urm_alt_m) {

  // 各電装の気圧高度を物理計算
  data_air_bmp_altitude_m = calculate_bmp_altitude(air_press_hPa, air_temp_deg);
  float under_bmp_alt = calculate_bmp_altitude(under_press_hPa, under_temp_deg);
  float fslg_bmp_alt = calculate_bmp_altitude(fslg_press_hPa, fslg_temp_deg);

  // 移動平均フィルタに追加
  filtered_air_bmp_altitude_m.add(data_air_bmp_altitude_m);
  filtered_under_bmp_altitude_m.add(under_bmp_alt);
  filtered_fslg_bmp_altitude_m.add(fslg_bmp_alt);

  // 離陸前はプラットフォーム上の平均高度を更新する
  if (takeoff == false) {
    air_bmp_altitude_platform_m.add(data_air_bmp_altitude_m);
    under_bmp_altitude_platform_m.add(under_bmp_alt);
    fslg_bmp_altitude_platform_m.add(fslg_bmp_alt);
  }

  // (現在の高度) - (プラットフォーム上の平均高度) + (プラホの高度)
  float bmp_altitude_lake_array_m[3];
  bmp_altitude_lake_array_m[0] = filtered_air_bmp_altitude_m.get() - air_bmp_altitude_platform_m.get() + const_platform_altitude_m;
  bmp_altitude_lake_array_m[1] = filtered_under_bmp_altitude_m.get() - under_bmp_altitude_platform_m.get() + const_platform_altitude_m;
  bmp_altitude_lake_array_m[2] = filtered_fslg_bmp_altitude_m.get() - fslg_bmp_altitude_platform_m.get() + const_platform_altitude_m;

  // 3つの気圧高度の中央値を採用してグローバル変数へ代入
  filtered_bmp_altitude_m = bmp_altitude_lake_m.median(bmp_altitude_lake_array_m, 3);
  /* 気圧高度計算ここまで */

  /* 超音波高度フィルタリング */
  filtered_under_urm_altitude_m.add(under_urm_alt_m);

  // 超音波高度が信頼できるか判別
  if (under_urm_alt_m > 8.0f) {
    urm_is_reliable = false;
    filtered_urm_altitude_m = 0.0f;
  } else {
    urm_is_reliable = true;
    filtered_urm_altitude_m = filtered_under_urm_altitude_m.get();
  }

}

float filtered_tsd20 = 0.0f;

bool is_takeoff(){
    if (data_under_tsd20_altitude_m >= 0.0){
        filtered_tsd20_altitude_m.add(data_under_tsd20_altitude_m);
    }

    if (filtered_tsd20_altitude_m.get() > 3.0f) { // TSD20のフィルタリング済み高度が3.0m以上なら離陸と判定

        //for debug
        filtered_tsd20 = filtered_tsd20_altitude_m.get();

        return true;
    } else {
        return false;
    }
}