/*-----------------------

このファイルの役割：電流電圧測定

------------------------*/

#include "power_checker.h"
#include "Air_xiao_config.h"
#include <Arduino.h>

// 使用抵抗の定数（電流測定，LT6106用）
const float R_SENSE = 0.1; // シャント抵抗
const float R_IN = 100.0; // 入力抵抗
const float R_OUT = 2000.0; // 出力抵抗

// 使用抵抗の定数（電圧測定用）
const float R1 = 10 * 1000.0; // 10kΩ
const float R2 = 1.8 * 1000.0; // 1.8kΩ

void init_PowerChecker(){
    // ADCの減衰率を6dBに設定
    analogSetAttenuation(ADC_6db);
}


float read_voltage_V(){
    uint32_t V_ADC_mv = analogReadMilliVolts(Power_Checker_VOLTAGE);
    // 分圧回路の出力電圧を元の電圧に変換
    float V_ADC_v = V_ADC_mv / 1000.0;
    float V_input_v = V_ADC_v * (R1 + R2) / R2; // 分圧回路の出力電圧からバッテリー電圧を算出

    return V_input_v;
}



// 電流測定．mA単位で返す
float read_current_mA(){
    uint32_t V_OUT_mv = analogReadMilliVolts(Power_Checker_CURRENT);
    float V_OUT_v = V_OUT_mv / 1000.0;

    // 負荷に流れる電流を算出
    // I_LOAD = V_OUT * (R_IN / (R_OUT * R_SENSE) )
    float I_LOAD_A = V_OUT_v * (R_IN / (R_OUT * R_SENSE) );
    float I_LOAD_mA = I_LOAD_A * 1000.0;

    return I_LOAD_mA;
}