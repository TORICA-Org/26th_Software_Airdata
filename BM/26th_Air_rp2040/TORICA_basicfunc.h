//各種関数の宣言

#pragma once // インクルードガード（複数回読み込まれないようにする）

//setup関数内で使うもの


//LED初期化
void LED_init(void);

//USBケーブルを差したときの起動猶予．処理を止めたいのでdelay関数使う
void init_delay_10sec(void);



//loop関数内で使うもの

//対気速度の計算
float calc_airspeed_ms(float sdp_differentialPressure_Pa, float dps_pressure_hPa, float dps_temperature_deg)


//気圧高度の計算
float calc_pressureAltitude_m(float dps_pressure_hPa, float dps_temperature_deg);


