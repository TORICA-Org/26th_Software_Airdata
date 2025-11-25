#define DEBUG_MODE  //デバッグモード

#include <Arduino.h>
#include <TORICA_parameters.h>
#include <TORICA_basicfunc.h>

//UARTの宣言
#define SerialAir_xiao Serial1
#define SerialUnder Serial2
SerialPIO Serial_ICS(Serial_ICS_TX, Serial_ICS_RX);  //ICS基盤用
//SerialPIO Serial_GPS(Serial_GPS_TX, Serial_GPS_RX); //GPS用


//ライブラリ読み込みと初期化とか

//GPS
#include <TinyGPSPlus.h>
TinyGPSPlus gps;

//TORICA_UARTインスタンス化
#include <TORICA_UART.h>
TORICA_UART Under_UART(&SerialUnder);
TORICA_UART Air_xiao_UART(&SerialAir_xiao);

//TORICA_ICS初期化
#include <TORICA_ICS.h>
TORICA_ICS ics(&Serial_ICS);

//I2Cパッケージ
#include <Wire.h>

#include <TORICA_SDP810.h>
#include <TORICA_BMP3XX.h>
#include <TORICA_BNO055.h>
#include <TORICA_AS5600.h>
#include <TORICA_MoveAve.h>


// // 対気速度
// TORICA_MoveAve<5> filtered_airspeed_ms(0);
// // 現在の気圧高度(気圧基準)
// TORICA_MoveAve<5> filtered_air_bmp_altitude_m(0);
// TORICA_MoveAve<5> filtered_under_bmp_altitude_m(0);
// // プラホの高度(気圧基準)
// TORICA_MoveAve<50> air_bmp_altitude_platform_m(0);
// TORICA_MoveAve<50> under_bmp_altitude_platform_m(0);
// // 気圧センサを用いた信頼できる対地高度
// // 3つの気圧高度にそれぞれ移動平均をとってプラホを10mとし，中央値をとった値
// #include <QuickStats.h>
// float bmp_altitude_lake_array_m[3];
// QuickStats bmp_altitude_lake_m;
// // 超音波高度(対地高度)
// TORICA_MoveAve<3> filtered_under_urm_altitude_m(0);
// #include<TORICA_MoveMedian.h>
// // 気圧での対地高度と超音波での対地高度の差
// // 100Hz(calculate)*4s = 400
// TORICA_MoveMedian<400> altitude_bmp_urm_offset_m(0);


//100Hz周期実行用
class Timer {
public:
  void setInterval(int _interval) {
    interval = _interval;
  }

  void run(void (*function)()) {
    // 引数([戻り値の型] *([ポインタ変数名])([引数情報]))
    if (millis() - last_timestamp >= interval) {
      last_timestamp = millis();
      function();
    }
  }

private:
  int interval = 0;
  unsigned long last_timestamp = 0;
};

Timer Timer1;
Timer Timer2;


void setup() {
  LED_init();  //LED初期動作
  Serial.begin(460800);

  SerialAir_xiao.setFIFOSize(1024);
  SerialAir_xiao.setTX(SerialAir_xiao_TX);
  SerialAir_xiao.setRX(SerialAir_xiao_RX);
  SerialAir_xiao.begin(460800);

  SerialUnder.setFIFOSize(1024);
  SerialUnder.setTX(SerialUnder_TX);
  SerialUnder.setRX(SerialUnder_RX);
  SerialUnder.begin(460800);

  Wire.setSDA(bico_I2C0_SDA);
  Wire.setSCL(bico_I2C0_SCL);
  Wire1.setSDA(bico_I2C1_SDA);
  Wire1.setSCL(bico_I2C1_SCL);
  Wire.begin();
  Wire1.begin();
  Wire.setClock(400000);
  Wire1.setClock(400000);

  init_delay_10sec();
  LED_init();  //LED初期動作


  TORICA_SDP810_init(Wire, 0x25);
  TORICA_AS5600_AoS_init(Wire, 0x36);
  TORICA_BMP3XX_init(Wire1, 0x77);
  TORICA_BNO055_init(Wire1, 0x28);
  TORICA_AS5600_AoA_init(Wire1, 0x36);


//DEBUG_MODEが有効の時のみ実行
#ifdef DEBUG_MODE
  init_delay_10sec();  //USBケーブルを差したときの起動猶予．処理を止めたいのでdelay関数使う
  Serial.println("Debug Mode Enabled");
#endif  //DEBUG_MODE

  //100Hz周期実行用
  Timer1.setInterval(10);
}


void loop() {
  //100Hz周期で実行
  Timer1.run([]() -> void {
    //AS5600 AoA,AoS取得
    digitalWrite(LED_ICS, HIGH);
    data_air_AoS_angle_deg = TORICA_AS5600_getAoS();
    data_air_AoA_angle_deg = TORICA_AS5600_getAoA();
    digitalWrite(LED_ICS, LOW);

    //BNO055
    digitalWrite(LED_Under, HIGH);
    TORICA_BNO055_rollpitchyaw(data_air_bno_roll, data_air_bno_pitch, data_air_bno_yaw);
    TORICA_BNO055_acc(data_air_bno_accx_mss, data_air_bno_accy_mss, data_air_bno_accz_mss);
    TORICA_BNO055_quaternion(data_air_bno_qw, data_air_bno_qx, data_air_bno_qy, data_air_bno_qz);
    TORICA_BNO055_getCalibration(data_air_bno_cal_system, data_air_bno_cal_gyro, data_air_bno_cal_accel, data_air_bno_cal_mag);
    digitalWrite(LED_Under, LOW);

    //BMP390
    digitalWrite(LED_Air_pico, HIGH);
    data_air_bmp_temperature_deg = TORICA_BMP3XX_getTemperature_deg();
    data_air_bmp_pressure_hPa = TORICA_BMP3XX_getPressure_hPa();
    data_air_bmp_altitude_m = calc_pressureAltitude_m(data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg);
    digitalWrite(LED_Air_pico, LOW);

    //SDP測定と対気速度計算
    digitalWrite(LED_Air_xiao, HIGH);
    data_air_sdp_differentialPressure_Pa = TORICA_SDP810_getdifferentialPressure_Pa();
    data_air_sdp_airspeed_ms = calc_airspeed_ms(data_air_sdp_differentialPressure_Pa, data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg);
    digitalWrite(LED_Air_xiao, LOW);


//シリアル出力
#ifdef DEBUG_MODE
    Serial.print("AoS_angle_deg:");
    Serial.print(data_air_AoS_angle_deg);
    Serial.print("\t");
    Serial.print("AoA_angle_deg:");
    Serial.print(data_air_AoA_angle_deg);
    Serial.print("\t");
    Serial.print("Roll:");
    Serial.print(data_air_bno_roll);
    Serial.print("\t");
    Serial.print("Pitch:");
    Serial.print(data_air_bno_pitch);
    Serial.print("\t");
    Serial.print("Yaw:");
    Serial.print(data_air_bno_yaw);
    Serial.print("\t");
    Serial.print("BMP_Temp:");
    Serial.print(data_air_bmp_temperature_deg);
    Serial.print("\t");
    Serial.print("BMP_Pres:");
    Serial.print(data_air_bmp_pressure_hPa);
    Serial.print("\t");
    Serial.print("BMP_Alt:");
    Serial.print(data_air_bmp_altitude_m);
    Serial.print("\t");
    Serial.print("Airspeed:");
    Serial.print(data_air_sdp_airspeed_ms);
    Serial.println();
#endif  //DEBUG_MODE

    digitalWrite(LED_GPS, HIGH);
    send_data(SerialAir_xiao, "air_xiao");  //テレメトリ用にair_xiaoへ送信
    digitalWrite(LED_GPS, LOW);
  });
}