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
static char value_voltage_current[16];

// bno
static constexpr char label_bno_calib[] = "bno_calib: s,g,a,m";
static char value_bno_calib[16];
static constexpr char label_bno_eular[] = "bno_eular: roll,pitch,yaw";
static char value_bno_eular[32];

// bmp
static constexpr char label_bmp[] = "bmp_alt: air, under, fslg";
static char value_bmp[16];

// URM, TSD20
static constexpr char label_urm_tsd[] = "URM, TSD20: dist_air, dist_under";
static char value_urm_tsd[32];

// sdp
static constexpr char label_airspeed[] = "airspd";
static char value_airspeed[16];

// AoA,AoS
static constexpr char label_AoA_AoS[] = "AoA, AoS";
static char value_AoA_AoS[16];

// GPS
static constexpr char label_gps[] = "gps: lat, lon";
static char value_gps[32];

// ICS_angle
static constexpr char label_ics_angle[] = "ICS_angle";
static char value_ics_angle[16];


void sendSerialWeb(){

    // time_ms
    sprintf(value_time_ms, "%d", time_ms);
    SerialWeb.send(label_time_ms, value_time_ms);

    // 電流電圧
    sprintf(value_voltage_current, "%.2f, %.2f", read_voltage_V(), read_current_mA());
    SerialWeb.send(label_voltage_current, value_voltage_current);

    // bnoのキャリブレーション状態
    sprintf(value_bno_calib, "%u, %u, %u, %u", data_fslg_bno_cal_system, data_fslg_bno_cal_gyro, data_fslg_bno_cal_accel, data_fslg_bno_cal_mag);
    SerialWeb.send(label_bno_calib, value_bno_calib);

    // bnoのroll,pitch,yaw
    sprintf(value_bno_eular, "%.2f, %.2f, %.2f", data_fslg_bno_roll, data_fslg_bno_pitch, data_fslg_bno_yaw);
    SerialWeb.send(label_bno_eular, value_bno_eular);

    // bmpの高度(air,under,fslgの順)
    sprintf(value_bmp, "%.2f, %.2f, %.2f", data_air_bmp_altitude_m, data_under_bmp_altitude_m, data_fslg_bmp_altitude_m);
    SerialWeb.send(label_bmp, value_bmp);

    // URMとTSD20の高度(air, underの順)
    sprintf(value_urm_tsd, "%.2f, %.2f", data_under_urm_altitude_m, data_under_tsd20_altitude_m);
    SerialWeb.send(label_urm_tsd, value_urm_tsd);

    // airspeed
    sprintf(value_airspeed, "%.2f", data_air_sdp_airspeed_ms);
    SerialWeb.send(label_airspeed, value_airspeed);

    // AoA,AoS
    sprintf(value_AoA_AoS, "%.2f, %.2f", data_air_AoA_angle_deg, data_air_AoS_angle_deg);
    SerialWeb.send(label_AoA_AoS, value_AoA_AoS);

    // GPS lat,lon
    sprintf(value_gps, "%.7f, %.7f", data_air_gps_latitude_deg, data_air_gps_longitude_deg);
    SerialWeb.send(label_gps, value_gps);

    // ICS_angle
    sprintf(value_ics_angle, "%d", data_ics_angle);
    SerialWeb.send(label_ics_angle, value_ics_angle);
}