/*-----------------------

このファイルの役割：XiaoでのSD用関数

------------------------*/

#include "SD_Air_xiao.h"

//ピン配置定義ファイルを読み込む
#include "Air_xiao_config.h"

#include "parameters.h"


TORICA_SD sd; //引数なしでインスタンス化

char SD_BUF[256]; //SD書き込み用バッファ

//SD初期化コード
void initSD(){
    #ifdef ARDUINO_ARCH_RP2040 //RP2040およびRP2350のチェック用
    SPI.setCS(SD_CS);
    SPI.setSCK(SD_SCK);
    SPI.setTX(SD_MOSI);
    SPI.setRX(SD_MISO);
    #endif

    SPI.begin();
    sd.begin(SD_CS);
    
}


void flashHeader() {
  // この関数は`setup()`内なのでブロッキング関数（処理の流れが止まる関数）であっても構わない
  const char *str[3];

  for (int i = 0; i < 4 /* case0~3まで実行 */; i++) {

    switch (i) {
      case 0: // 11個
        { 
          str[0] = "time_ms,takeoff,urm_is_reliable,data_air_gps_hour,"; // 4個
          str[1] = "data_air_gps_minute,data_air_gps_second,data_air_gps_centisecond,data_air_gps_latitude_deg,"; // 4個
          str[2] = "data_air_gps_longitude_deg,data_air_gps_altitude_m,data_air_gps_groundspeed_ms,"; // 3個
          break;
        }
      case 1:
        { // 10個
          str[0] = "filtered_bmp_altitude_m,filtered_urm_altitude_m,data_air_bmp_pressure_hPa,data_air_bmp_temperature_deg,"; // 4個
          str[1] = "data_air_bmp_altitude_m,data_air_sdp_differentialPressure_Pa,data_air_sdp_airspeed_ms,"; // 3個
          str[2] = "data_air_AoA_angle_deg,data_air_AoS_angle_deg,data_ics_angle,"; // 3個
          break;
        }
      case 2: // 11個
        {
          str[0] = "psd_is_alive,data_psd_bno_qw,data_psd_bno_qx,data_psd_bno_qy,"; // 4個
          str[1] = "data_psd_bno_qz,data_psd_bno_roll,data_psd_bno_pitch,data_psd_bno_yaw,"; // 4個
          str[2] = "data_psd_bmp_pressure_hPa,data_psd_bmp_temperature_deg,data_psd_bmp_altitude_m,"; // 3個
          break;
        }
      case 3: // 13個
        {
          str[0] = "data_psd_bno_accx_mss,data_psd_bno_accy_mss,data_psd_bno_accz_mss,"; // 3個
          str[1] = "data_psd_bno_calibration,under_is_alive,data_under_bmp_pressure_hPa,"; // 3個
          str[2] = "data_under_bmp_temperature_deg,data_under_bmp_altitude_m,data_under_urm_altitude_m,data_under_tsd20_altitude_m\n"; // 4個
          break;
        }
      default:
        {
          Serial.println("The parameter value is out of range.");
          break;
        }
    }

    sprintf(SD_BUF, "%s%s%s", str[0], str[1], str[2]);

    sd.add_str(SD_BUF);
    sd.flash();

    delayMicroseconds(10);  // 遅延あったほうがいいと思う
  }
}


//とりあえず20Hz書き込みで様子見
void flashSD(int flash_mode){
    memset(SD_BUF, 0, sizeof(SD_BUF)); //SD_BUFを0で初期化

    switch (flash_mode){
        case 0: {
            sprintf(SD_BUF, "%lu,%d,%d,%u,%u,%u,%u,%.7f,%.7f,%.2f,%.2f,", 
                time_ms, takeoff, urm_is_reliable, data_air_gps_hour, // 4個
                data_air_gps_minute, data_air_gps_second, data_air_gps_centisecond, data_air_gps_latitude_deg, // 4個
                data_air_gps_longitude_deg, data_air_gps_altitude_m, data_air_gps_groundspeed_ms); // 3個
            sd.add_str(SD_BUF);
            sd.flash();
            break;
        }
        case 1: {
             sprintf(SD_BUF, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,", 
                filtered_bmp_altitude_m, filtered_urm_altitude_m, data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg, // 4個
                data_air_bmp_altitude_m, data_air_sdp_differentialPressure_Pa, data_air_sdp_airspeed_ms, // 3個
                data_air_AoA_angle_deg, data_air_AoS_angle_deg, data_ics_angle); // 3個
            sd.add_str(SD_BUF);
            sd.flash();
            break;
        }
        case 2: {
            sprintf(SD_BUF, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,", 
                psd_is_alive, data_psd_bno_qw, data_psd_bno_qx, data_psd_bno_qy, // 4個
                data_psd_bno_qz, data_psd_bno_roll, data_psd_bno_pitch, data_psd_bno_yaw, // 4個
                data_psd_bmp_pressure_hPa, data_psd_bmp_temperature_deg, data_psd_bmp_altitude_m); // 3個
            sd.add_str(SD_BUF);
            sd.flash();
            break;
        }
         case 3: {
            sprintf(SD_BUF, "%.2f,%.2f,%.2f,%u%u%u%u,%d,%.2f,%.2f,%.2f,%.2f,%.2f\n",  // BNOのキャリブレーションレベルはsystem,gyro,accel,magの順に一つのセルに格納．ex)1111
                data_psd_bno_accx_mss, data_psd_bno_accy_mss, data_psd_bno_accz_mss, data_psd_bno_cal_system, data_psd_bno_cal_gyro, // 5個
                data_psd_bno_cal_accel, data_psd_bno_cal_mag, under_is_alive, data_under_bmp_pressure_hPa, // 4個
                data_under_bmp_temperature_deg, data_under_bmp_altitude_m, data_under_urm_altitude_m, data_under_tsd20_altitude_m); // 4個
            sd.add_str(SD_BUF);
            sd.flash();
            break;
    }
    default: {
        Serial.println("Invalid argument");
    }
    }
}