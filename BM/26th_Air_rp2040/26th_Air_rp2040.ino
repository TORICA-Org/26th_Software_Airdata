#define DEBUG_MODE //デバッグモード

#include <Arduino.h> 
#include "parameters.h" 
#include "TORICA_basicfunc.h" 

//UARTの宣言
#define SerialAir_xiao Serial1
#define SerialUnder Serial2
SerialPIO Serial_ICS(10,11); //ICS基盤用
//SerialPIO SerialGPS(2,3); //GPS用


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

#include "sensors/TORICA_SDP31.h"
#include "sensors/TORICA_SDP31.cpp"
#include "sensors/TORICA_BMP3XX.h"
#include "sensors/TORICA_BMP3XX.cpp"
#include "sensors/TORICA_BNO055.h"
#include "sensors/TORICA_BNO055.cpp"
#include "sensors/TORICA_AS5600.h"
#include "sensors/TORICA_AS5600.cpp"


#include <TORICA_MoveAve.h>
// 対気速度
TORICA_MoveAve<5> filtered_airspeed_ms(0);
// 現在の気圧高度(気圧基準)
TORICA_MoveAve<5> filtered_air_bmp_altitude_m(0);
TORICA_MoveAve<5> filtered_under_bmp_altitude_m(0);
// プラホの高度(気圧基準)
TORICA_MoveAve<50> air_bmp_altitude_platform_m(0);
TORICA_MoveAve<50> under_bmp_altitude_platform_m(0);
// 気圧センサを用いた信頼できる対地高度
// 3つの気圧高度にそれぞれ移動平均をとってプラホを10mとし，中央値をとった値
#include <QuickStats.h>
float bmp_altitude_lake_array_m[3];
QuickStats bmp_altitude_lake_m;
// 超音波高度(対地高度)
TORICA_MoveAve<3> filtered_under_urm_altitude_m(0);
#include<TORICA_MoveMedian.h>
// 気圧での対地高度と超音波での対地高度の差
// 100Hz(calculate)*4s = 400
TORICA_MoveMedian<400> altitude_bmp_urm_offset_m(0);


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


void setup(){
  Serial.begin(460800);
  Serial1.setFIFOSize(1024);
  Serial1.setTX(0);
  Serial1.setRX(1);
  Serial1.begin(460800);
  Wire.setSDA(26);
  Wire.setSCL(27);
  Wire1.setSDA(28);
  Wire1.setSCL(29);
  Wire.begin();
  Wire1.begin();
  Wire.setClock(400000);
  Wire1.setClock(400000);

  init_delay_10sec();
  LED_init(); //LED初期動作


  TORICA_SDP31_init(Wire, 0x23);
  TORICA_BMP3XX_init(Wire1, 0x76);
  TORICA_BNO055_init(Wire1, 0x28);
  TORICA_AS5600_AoS_init(Wire, 0x36);
  TORICA_AS5600_AoA_init(Wire1, 0x36);
  

  //DEBUG_MODEが有効の時のみ実行
  #ifdef DEBUG_MODE
    init_delay_10sec(); //USBケーブルを差したときの起動猶予．処理を止めたいのでdelay関数使う
    Serial.println("Debug Mode Enabled");
  #endif //DEBUG_MODE

  //100Hz周期実行用
  Timer1.setInterval(10);
  Adafruit_BNO055 *TORICA_BNO055_get();
  Adafruit_BMP3XX *TORICA_BMP3XX_get();
}


void loop(){
  //100Hz周期で実行
  Timer1.run([]() -> void {

    //AS5600 AoA,AoS取得
    data_air_AoA_angle_deg = TORICA_AS5600_getAoS();
    data_air_AoA_angle_deg = TORICA_AS5600_getAoA();

    //BNO055
    TORICA_BNO055_rollpitchyaw(data_air_bno_roll, data_air_bno_pitch, data_air_bno_yaw);
    TORICA_BNO055_acc(data_air_bno_accx_mss, data_air_bno_accy_mss, data_air_bno_accz_mss);
    TORICA_BNO055_quaternion(data_air_bno_qw, data_air_bno_qx, data_air_bno_qy, data_air_bno_qz);
    //TORICA_BNO055_getCalibration(sys_calib, gyro_calib, accel_calib, mag_calib);

    //BMP390
    data_air_bmp_temperature_deg = TORICA_BMP3XX_getTemperature();
    data_air_bmp_pressure_hPa = TORICA_BMP3XX_getPressure();
    data_air_bmp_altitude_m = calc_pressureAltitude_m(data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg);

    //SDP測定と期待速度計算
    data_air_sdp_differentialPressure_Pa = TORICA_SDP31_getdifferentialPressure_Pa();
    data_air_sdp_airspeed_ms = calc_airspeed_ms(data_air_sdp_differentialPressure_Pa, data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg);

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
    #endif //DEBUG_MODE
  });
}



/*-------------------

void SD_setting() {
  static int loop_count = 0;
  if (loop_count == 0) {
    sprintf(UART_SD, "time_ms, data_air_bno_accx_mss, data_air_bno_accy_mss, data_air_bno_accz_mss, data_air_bno_qw, data_air_bno_qx, data_air_bno_qy, data_air_bno_qz, data_air_bno_roll, data_air_bno_pitch,"); //10個
  } else if (loop_count == 1) {
    sprintf(UART_SD, "data_air_bno_yaw, estimated_altitude_lake_m, altitude_bmp_urm_offset_m.get(), flight_phase, speed_level, data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg, data_air_bmp_altitude_m, data_under_bmp_pressure_hPa, data_under_bmp_temperature_deg,"); //10個
  } else if (loop_count == 2) {
    sprintf(UART_SD, "data_under_bmp_altitude_m, data_under_urm_altitude_m, data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg, data_air_bmp_altitude_m, data_air_sdp_differentialPressure_Pa, data_air_sdp_airspeed_ms, data_air_AoA_angle_deg, data_air_AoS_angle_deg,"); //10個
  } else if (loop_count == 3) {
    sprintf(UART_SD, "data_ics_angle, data_air_bno_accx_mss, data_air_bno_accy_mss, data_air_bno_accz_mss, data_air_bno_qw, data_air_bno_qx, data_air_bno_qy, data_air_bno_qz, data_air_bno_roll, data_air_bno_pitch, data_air_bno_yaw,"); //10個
  } else { 
    sprintf(UART_SD, "data_air_gps_hour, data_air_gps_minute, data_air_gps_second, data_air_gps_centisecond, data_air_gps_latitude_deg, data_air_gps_longitude_deg, data_air_gps_altitude_m,data_air_gps_groundspeed_ms\n"); //8個
  }
  loop_count++;
  SerialAir_xiao.flush();
  SerialUnder.flush();
  SerialAir_xiao.print(UART_SD);
  SerialUnder.print(UART_SD);
}



void send_SD() {
  time_ms = millis();
  static int loop_count = 0;
  static uint32_t SD_last_send_time = 0;
  if (loop_count == 0) {
    sprintf(UART_SD, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,",  //10個
            time_ms, data_air_bno_accx_mss, data_air_bno_accy_mss, data_air_bno_accz_mss, 
            data_air_bno_qw, data_air_bno_qx, data_air_bno_qy, data_air_bno_qz, 
            data_air_bno_roll, data_air_bno_pitch);
  } else if (loop_count == 1) {
    sprintf(UART_SD, "%.2f,%.2f,%.2f,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,", //10個
            data_air_bno_yaw, estimated_altitude_lake_m, altitude_bmp_urm_offset_m.get(), flight_phase, speed_level,
            data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg, data_air_bmp_altitude_m, data_under_bmp_pressure_hPa,
            data_under_bmp_temperature_deg);
  } else if (loop_count == 2) {
    sprintf(UART_SD, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,",  //10個
            data_under_bmp_altitude_m, data_under_urm_altitude_m, data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg, data_air_bmp_altitude_m,
            data_air_sdp_differentialPressure_Pa, data_air_sdp_airspeed_ms, data_air_AoA_angle_deg,
            data_air_AoS_angle_deg, data_ics_angle);
  } else if (loop_count == 3) {
    sprintf(UART_SD, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,",   //10個
            data_air_bno_accx_mss, data_air_bno_accy_mss, data_air_bno_accz_mss,
            data_air_bno_qw, data_air_bno_qx, data_air_bno_qy, data_air_bno_qz,
            data_air_bno_roll, data_air_bno_pitch, data_air_bno_yaw);
  } else {
    sprintf(UART_SD, "%u,%u,%u,%u,%10.7lf,%10.7lf,%5.2lf,%5.2lf\n",    //8個
            data_air_gps_hour, data_air_gps_minute, data_air_gps_second, data_air_gps_centisecond,
            data_air_gps_latitude_deg, data_air_gps_longitude_deg, data_air_gps_altitude_m,data_air_gps_groundspeed_ms);
    loop_count = -1;
  }
  SD_last_send_time = millis();
  loop_count++;
  //バッファをクリアしてから新しいデータを書き込み
  SerialAir_xiao.flush();
  SerialUnder.flush();
  SerialAir_xiao.print(UART_SD);
  SerialUnder.print(UART_SD);
}



------------------*/
