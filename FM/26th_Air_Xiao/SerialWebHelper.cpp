#include "SerialWebHelper.h"
#include "power_checker.h" // 電流電圧読み取り用

void initSerialWeb() {
  SerialWeb.begin(SSID, PASSWORD);  // Serialなどと同様に初期化します．
}



/* sendSerialWeb()用変数宣言 */
// time_ms
static constexpr char label_time_ms[] = "time_ms";
static char value_time_ms[32];

// 電流電圧計
static constexpr char label_voltage_current[] = "Volt, mA";
static char value_voltage_current[32];

// bno
static constexpr char label_bno_calib[] = "BNO_calib: s,g,a,m";
static char value_bno_calib[32];
static constexpr char label_bno_eular[] = "BNO_eular: roll, pitch, yaw";
static char value_bno_eular[32];

// lsm
static constexpr char label_lsm_eular[] = "LSM_eular: roll, pitch, yaw";
static char value_lsm_eular[32];

// bmp temperature
static constexpr char label_bmp1[] = "bmp_temp: air, under, fslg";
static char value_bmp1[32];

// bmp altitude
static constexpr char label_bmp2[] = "bmp_alt: air, under, fslg";
static char value_bmp2[32];

// URM, TSD20
static constexpr char label_urm_tsd[] = "URM, TSD20";
static char value_urm_tsd[32];

// sdp
static constexpr char label_airspeed[] = "airspd";
static char value_airspeed[32];

// AoA,AoS
static constexpr char label_AoA_AoS[] = "AoA, AoS";
static char value_AoA_AoS[32];

// GPS緯度経度
static constexpr char label_gps1[] = "GPS: lat, lon";
static char value_gps1[32];

// GPS衛星補足数
static constexpr char label_gps2[] = "GPS: satellites";
static char value_gps2[8];

// ICS_angle
static constexpr char label_ics_angle[] = "ICS_angle";
static char value_ics_angle[32];


void sendSerialWeb(){

    // time_ms
    snprintf(value_time_ms, sizeof(value_time_ms) , "%lu", time_ms);
    SerialWeb.send(label_time_ms, value_time_ms);

    // 電流電圧
    snprintf(value_voltage_current, sizeof(value_voltage_current),"%.2f, %.2f", read_voltage_V(), read_current_mA());
    SerialWeb.send(label_voltage_current, value_voltage_current);

    // bnoのキャリブレーション状態
    snprintf(value_bno_calib, sizeof(value_bno_calib), "%u, %u, %u, %u", data_fslg_bno_cal_system, data_fslg_bno_cal_gyro, data_fslg_bno_cal_accel, data_fslg_bno_cal_mag);
    SerialWeb.send(label_bno_calib, value_bno_calib);

    // bnoのroll,pitch,yaw
    // snprintf(value_bno_eular, sizeof(value_bno_eular), "%.2f, %.2f, %.2f", data_fslg_bno_roll, data_fslg_bno_pitch, data_fslg_bno_yaw);
    // floatの文字列変換って結構スタックメモリ食うので，整数部と小数部で分けて文字列化しています
    snprintf(value_bno_eular, sizeof(value_bno_eular), "%d.%02d, %d.%02d, %d.%02d",
             (int)data_fslg_bno_roll, abs((int)(data_fslg_bno_roll * 100) % 100),
             (int)data_fslg_bno_pitch, abs((int)(data_fslg_bno_pitch * 100) % 100),
             (int)data_fslg_bno_yaw, abs((int)(data_fslg_bno_yaw * 100) % 100));
    SerialWeb.send(label_bno_eular, value_bno_eular);

    // LSM 6軸IMU
    snprintf(value_lsm_eular, sizeof(value_lsm_eular), "%d.%02d, %d.%02d, %d.%02d",
             (int)data_fslg_lsm_roll, abs((int)(data_fslg_lsm_roll * 100) % 100),
             (int)data_fslg_lsm_pitch, abs((int)(data_fslg_lsm_pitch * 100) % 100),
             (int)data_fslg_lsm_yaw, abs((int)(data_fslg_lsm_yaw * 100) % 100));
    SerialWeb.send(label_lsm_eular, value_lsm_eular);

    // bmp温度
    snprintf(value_bmp1, sizeof(value_bmp1), "%d.%02d, %d.%02d, %d.%02d",
             (int)data_air_bmp_temperature_deg, abs((int)(data_air_bmp_temperature_deg * 100) % 100),
             (int)data_under_bmp_temperature_deg, abs((int)(data_under_bmp_temperature_deg * 100) % 100),
             (int)data_fslg_bmp_temperature_deg, abs((int)(data_fslg_bmp_temperature_deg * 100) % 100));
    SerialWeb.send(label_bmp1, value_bmp1);


    // bmpの高度(air,under,fslgの順)
    // snprintf(value_bmp, sizeof(value_bmp), "%.2f, %.2f, %.2f", data_air_bmp_altitude_m, data_under_bmp_altitude_m, data_fslg_bmp_altitude_m);
    snprintf(value_bmp2, sizeof(value_bmp2), "%d.%02d, %d.%02d, %d.%02d",
             (int)data_air_bmp_altitude_m, abs((int)(data_air_bmp_altitude_m * 100) % 100),
             (int)data_under_bmp_altitude_m, abs((int)(data_under_bmp_altitude_m * 100) % 100),
             (int)data_fslg_bmp_altitude_m, abs((int)(data_fslg_bmp_altitude_m * 100) % 100));
    SerialWeb.send(label_bmp2, value_bmp2);
    

    // URMとTSD20の高度(air, underの順)
    snprintf(value_urm_tsd, sizeof(value_urm_tsd), "%.2f, %.2f", data_under_urm_altitude_m, data_under_tsd20_altitude_m);
    SerialWeb.send(label_urm_tsd, value_urm_tsd);

    // airspeed
    snprintf(value_airspeed, sizeof(value_airspeed), "%.2f", data_air_sdp_airspeed_ms);
    SerialWeb.send(label_airspeed, value_airspeed);

    // AoA,AoS
    snprintf(value_AoA_AoS, sizeof(value_AoA_AoS), "%.2f, %.2f", data_air_AoA_angle_deg, data_air_AoS_angle_deg);
    SerialWeb.send(label_AoA_AoS, value_AoA_AoS);

    // GPS lat,lon
    snprintf(value_gps1, sizeof(value_gps1), "%.7f, %.7f", data_air_gps_latitude_deg, data_air_gps_longitude_deg);
    SerialWeb.send(label_gps1, value_gps1);

    // GPS衛星補足数
    snprintf(value_gps2, sizeof(value_gps2), "%u", data_air_gps_satellites);
    SerialWeb.send(label_gps2, value_gps2);

    // ICS_angle
    snprintf(value_ics_angle, sizeof(value_ics_angle), "%d", data_ics_angle);
    SerialWeb.send(label_ics_angle, value_ics_angle);
}

void SerialWeb_detectRESET(){
  if (SerialWeb.available() >= 5) { // 5文字以上の受信を判定

    char resetBuf[16];
    int len = SerialWeb.readBytes(resetBuf, sizeof(resetBuf) - 1);
    resetBuf[len] = '\0'; // 文末処理

    // 文字列のトリミング．改行などを消す．
    for(int i = 0; i < len; i++) {
      if(resetBuf[i] == '\r' || resetBuf[i] == '\n') resetBuf[i] = '\0';
    }

    // 大文字小文字を無視して"RESET"判定
    if (strcasecmp(resetBuf, "RESET") == 0) { 
      SerialWeb.println("=====RESET====="); // Webページの"Log"タブに表示
      Serial1.println("RESET");
      RESET_SIG = true;
    }
  }
}