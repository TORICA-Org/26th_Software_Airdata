#include "calculate_airspeed.h"
#include <TORICA_MoveAve.h>

TORICA_MoveAve<5> filtered_airspeed(0); // 直近5回で取得した対気速度の平均


void calculate_airspeed() {
    
    /* 対気速度の計算
    計算式：\sqrt{| 2 \Delta P \times \frac{T}{P} \times \frac{R}{M} |}
    ただし R=8.314 \times 10^3 [J/(kmol \cdot K)], M=28.966 [kg/kmol] より R/M=287.026 [J/(kg \cdot K)] として計算
    */

    // 対気速度の計算にSDPとBMPの値を使うので，BMPとSDPの値取得後に計算
    data_air_sdp_airspeed_ms = sqrt(abs(2.0 * data_air_sdp_differentialPressure_Pa * ((data_air_bmp_temperature_deg + 273.15) / (data_air_bmp_pressure_hPa * 100.0)) * 287.026));
    filtered_airspeed.add(data_air_sdp_airspeed_ms);
    filtered_airspeed_ms = filtered_airspeed.get();
}