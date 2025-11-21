//各種関数の宣言

#pragma once  // インクルードガード（複数回読み込まれないようにする）

#include <Arduino.h>

//TORICA_ICSとTORICA_UARTの前方宣言
class TORICA_ICS;
class TORICA_UART;




//filtered用変数のextern宣言
#include <TORICA_MoveAve.h>
#include <TORICA_MoveMedian.h>
extern TORICA_MoveAve<5> filtered_airspeed_ms;
extern TORICA_MoveAve<5> filtered_air_bmp_altitude_m;
extern TORICA_MoveAve<5> filtered_under_bmp_altitude_m;
extern TORICA_MoveAve<50> air_bmp_altitude_platform_m;
extern TORICA_MoveAve<50> under_bmp_altitude_platform_m;
extern TORICA_MoveAve<3> filtered_under_urm_altitude_m;
extern TORICA_MoveMedian<400> altitude_bmp_urm_offset_m;




//setup関数内で使うもの


//LED初期化
void LED_init(void);

//USBケーブルを差したときの起動猶予．処理を止めたいのでdelay関数使う
void init_delay_10sec(void);



//loop関数内で使うもの

//対気速度の計算
float calc_airspeed_ms(float sdp_differentialPressure_Pa, float dps_pressure_hPa, float dps_temperature_deg);


//気圧高度の計算
float calc_pressureAltitude_m(float dps_pressure_hPa, float dps_temperature_deg);

//UART送信関連
bool SD_setting(Stream &serialPort, const char *option /*マイコン名を書く．*/);
bool send_data(Stream &serialPort, const char *option /*マイコン名を書く*/);

//bico向けとテレメトリ用xiao向け

bool polling_UART(const char *option /*送信元のマイコン名*/, TORICA_ICS *ICS = nullptr, TORICA_UART *Under = nullptr, TORICA_UART *air_rp2040 = nullptr);
//使用例
//poling_UART("air_rp2040",&ics,&Under) or polling_UART("air_xiao",air_rp2040 = &bico_UART);





//要検討！！！！
//flight phase決定
//void determine_flight_phase(void);
