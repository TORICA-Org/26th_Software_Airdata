/*
 * XIAO ESP32S3 + LT6106
*/

#define SENSE_PIN D1 // XIAO ESP32S3のD1

//使用抵抗の定数
const float V_SOURCE_v = 3.30;  // XIAOの3.3Vピンの電圧
const float R_SENSE  = 51.0;  // シャント抵抗
const float R_IN     = 990.0; // 入力抵抗
const float R_OUT    = 388.0; // 出力抵抗

void setup() {
  Serial.begin(115200);
  
  // ADCの減衰率を6dBに設定
  analogSetAttenuation(ADC_6db);
  
  Serial.println("XIAO ESP32S3 ADC Initialized");
}

void loop() {
  // v_OUTをmVで直接取得
  uint32_t V_OUT_mv = analogReadMilliVolts(SENSE_PIN);
  float V_OUT_v = V_OUT_mv / 1000.0;

  // I_LOADを算出
  // I_LOAD = V_OUT * (R_IN / (R_OUT * R_SENSE) )
  float I_LOAD_A = V_OUT_v * (R_IN / (R_OUT * R_SENSE) );
  float I_LOAD_mA = I_LOAD_A * 1000.0;

  // シャント抵抗での電圧降下 (Vsense)
  float V_SENSE_v = V_OUT_v * R_IN / R_OUT;

  // 負荷にかかっている電圧 (V_LOAD_V) = 電源電圧 - Rsenseでの電圧降下
  float V_LOAD_v = V_SOURCE_v - V_SENSE_v;

  Serial.printf("Vout: %4dmV | Current: %5.2fmA | Load Voltage: %5.3fV\n", 
                V_OUT_mv, I_LOAD_mA, V_LOAD_v);

  delay(100);
}