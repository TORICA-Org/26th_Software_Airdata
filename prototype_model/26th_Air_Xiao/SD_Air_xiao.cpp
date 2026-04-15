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


//とりあえず20Hz書き込みで様子見
void flashSD(int flash_mode){
    memset(SD_BUF, 0, sizeof(SD_BUF)); //SD_BUFを0で初期化

    switch (flash_mode){
        case 0: {
            sprintf(SD_BUF, "%lu,%d,%d,%.7f,%.7f,%.2f,%.2f,%u,%u,%u,%u",  // 11個
        time_ms, takeoff, speed_level, 
        data_air_gps_latitude_deg, data_air_gps_longitude_deg, 
        data_air_gps_altitude_m, data_air_gps_groundspeed_ms,
        data_air_gps_hour, data_air_gps_minute, data_air_gps_second, data_air_gps_centisecond);
            sd.add_str(SD_BUF);
            sd.flash();
            break;
        }
        case 1: {
             sprintf(SD_BUF, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d", // 10個
        estimated_altitude_lake_m, data_altitude_bmp_urm_offset_m,
        data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg, data_air_bmp_altitude_m,
        data_air_sdp_differentialPressure_Pa, data_air_sdp_airspeed_ms,
        data_air_AoA_angle_deg, data_air_AoS_angle_deg, data_ics_angle);

             sd.add_str(SD_BUF);
             sd.flash();
             break;
        }
        case 2: {
            sprintf(SD_BUF, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", // 10個
        data_psd_bmp_pressure_hPa, data_psd_bmp_temperature_deg, data_psd_bmp_altitude_m,
        data_psd_bno_accx_mss, data_psd_bno_accy_mss, data_psd_bno_accz_mss,
        data_psd_bno_qw, data_psd_bno_qx, data_psd_bno_qy, data_psd_bno_qz);
            sd.add_str(SD_BUF);
            sd.flash();
            break;
        }
         case 3: {
            sprintf(SD_BUF, "%.2f,%.2f,%.2f,%u,%u,%u,%u,%.2f,%.2f,%.2f,%.2f,%.2f\n", // 12個
        data_psd_bno_roll, data_psd_bno_pitch, data_psd_bno_yaw,
        data_psd_bno_cal_system, data_psd_bno_cal_gyro, data_psd_bno_cal_accel, data_psd_bno_cal_mag,
        data_under_bmp_pressure_hPa, data_under_bmp_temperature_deg, 
        data_under_bmp_altitude_m, data_under_urm_altitude_m, data_under_tsd20_altitude_m);
            sd.add_str(SD_BUF);
            sd.flash();
            break;
    }
    default: {
        Serial.println("Invalid argument");
    }
    }
}