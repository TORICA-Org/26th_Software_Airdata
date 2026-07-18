#include "calculate_airspeed.h"
#include "parameters.h"
#include <TORICA_MoveAve.h>

static TORICA_MoveAve<5> filtered_airspeed(0); // 直近5回で取得した対気速度の平均

// 対気速度補正式 (y = ax^2 + bx + c) の係数定義
const float CALIB_A = 0.0f; // x^2 の係数
const float CALIB_B = 0.0f; // x の係数
const float CALIB_C = 0.0f; // 定数項

// 対気速度の二次式補正関数
float correct_airspeed(float raw_airspeed) {
    if (CALIB_A == 0.0f && CALIB_B == 0.0f && CALIB_C == 0.0f){
        return raw_airspeed;
    }
    return CALIB_A * raw_airspeed * raw_airspeed + CALIB_B * raw_airspeed + CALIB_C;
}

void calculate_airspeed(float diff_press_Pa, float temp_deg, float press_hPa) {
    // 物理的な対気速度の計算
    float measured_airspeed_ms = 0.0f;
    if (press_hPa > 0.0f) {
        /*
        対気速度の計算
        計算式：\sqrt{| 2 \Delta P \times \frac{T}{P} \times \frac{R}{M} |}
        ただし R=8.314 \times 10^3 [J/(kmol \cdot K)], M=28.966 [kg/kmol] より R/M=287.026 として計算
        */
        
        float T = temp_deg + 273.15f; // ℃ -> K
        float P = press_hPa * 100.0f; // hPa -> Pa
        measured_airspeed_ms = sqrt(fabs(2.0f * diff_press_Pa * (T / P) * 287.026f));
    }

    // 二次式による対気速度の補正計算
    float calibrated_airspeed_ms = correct_airspeed(measured_airspeed_ms);

    data_air_sdp_airspeed_ms = calibrated_airspeed_ms;

    // 移動平均フィルタへ追加してグローバルへ代入
    filtered_airspeed.add(data_air_sdp_airspeed_ms);
    filtered_airspeed_ms = filtered_airspeed.get();
}