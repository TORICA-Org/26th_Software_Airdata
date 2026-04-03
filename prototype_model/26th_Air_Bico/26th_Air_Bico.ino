#define DEBUG_MODE  //デバッグモード

#include <Arduino.h>
#include "parameters.h"
#include "Bico_config.h"

//各ファイル読み込み
#include "calculate_altitude.h"
#include "AS5600.h"
#include "BMP3xx.h"
#include "BNO055.h"
#include "SDP810.h"
#include "UARTHelper_Bico.h"
#include "SD_Bico.h"


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

  //LED初期化
  pinMode(LED_ICS, OUTPUT);
  pinMode(LED_Under, OUTPUT);
  pinMode(LED_Air_pico, OUTPUT);
  pinMode(LED_Air_xiao, OUTPUT);
  pinMode(LED_GPS, OUTPUT);
  pinMode(LED_SD, OUTPUT);

  Serial.begin(460800, SERIAL_8E1); //DEBUG用USB-UART

  //ESP用・Under用UART初期化
  initUART();

  //SD内csv用ヘッダー送信
  transmitHeader();

  //Bico I2C0初期化動作
  Wire.setSDA(bico_I2C0_SDA);
  Wire.setSCL(bico_I2C0_SCL);
  Wire1.setSDA(bico_I2C1_SDA);
  Wire1.setSCL(bico_I2C1_SCL);
  Wire.begin();
  Wire1.begin();
  Wire.setClock(400000);
  Wire1.setClock(400000);
  

  //USB接続時のために起動待機（7秒）
  #ifdef DEBUG_MODE //DEBUG_MODEが有効ならば
  for (int i = 1; i <= 7; i++) {
    digitalWrite(LED_ICS, HIGH);
    digitalWrite(LED_Under, HIGH);
    digitalWrite(LED_Air_pico, HIGH);
    digitalWrite(LED_Air_xiao, HIGH);
    digitalWrite(LED_GPS, HIGH);
    digitalWrite(LED_SD, HIGH);
    delay(500);
    digitalWrite(LED_ICS, LOW);
    digitalWrite(LED_Under, LOW);
    digitalWrite(LED_Air_pico, LOW);
    digitalWrite(LED_Air_xiao, LOW);
    digitalWrite(LED_GPS, LOW);
    digitalWrite(LED_SD, LOW);
    delay(500);
  }
  Serial.println("DEBUG MODE Enabled");
  #endif //DEBUG_MODEが有効ならば


  SDP810_init();
  AS5600_init();
  BMP3XX_init();
  BNO055_init();

  Timer1.setInterval(10); //10ms(=100Hz)ごとにTimer1内の動作を実行
  Timer2.setInterval(10); //10ms(=100Hz)ごとにTimer2内の動作を実行
}


//CPU1のセットアップ
void setup1(){
  delay(3000); //とりあえずdelay挟んでいるが，特に意味は無い...
}


void loop() {

  func_100Hz();

}

void loop1() {

}



//UART送信，SD書き込み用カウント変数
int transmit_count = 0;
int flash_count = 0;

void func_100Hz() {
  //100Hz周期で実行
  Timer1.run([]() -> void {
    
    read_bmp_air();

    read_BNO();

    read_AS5600();

    read_SDP();

    /* 対気速度の計算
    計算式：\sqrt{| 2 \Delta P \times \frac{T}{P} \times \frac{R}{M} |}
    ただし R=8.314 \times 10^3 [J/(kmol \cdot K)], M=28.966 [kg/kmol] より R/M=287.026 [J/(kg \cdot K)] として計算
    */

    //対気速度の計算にSDPとBMPの値を使うので，BMPとSDPの値取得後に計算
    data_air_sdp_airspeed_ms = sqrt(abs(2.0 * data_air_sdp_differentialPressure_Pa * ((data_air_bmp_temperature_deg + 273.15) / (data_air_bmp_pressure_hPa * 100.0)) * 287.026));
    
    //機体下読み取り
    receiveLog();

    calculate_altitude();

    //UART送信とSD書き込み

    transmitLog(transmit_count);
    transmit_count++;
    //一通り送信したらカウントリセット
    if (transmit_count > 3) {
      transmit_count = 0;
    }

    flashSD(flash_count);
    flash_count++;
    //一通り書き込んだらカウントリセット
    if (flash_count > 3) {
      flash_count = 0;
    }    
    
  });
}