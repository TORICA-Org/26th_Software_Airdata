/*---------------------------

ファイルの役割：高度計算
最終更新日：2026/02/17 17:16
更新内容：ファイル作成

----------------------------*/

#pragma once

void calculate_altitude(float air_press_hPa, float air_temp_deg, float under_press_hPa, float under_temp_deg, float fslg_press_hPa, float fslg_temp_deg, float under_urm_alt_m);
float calculate_bmp_altitude(float pressure_hPa, float temperature_deg);

bool is_takeoff();