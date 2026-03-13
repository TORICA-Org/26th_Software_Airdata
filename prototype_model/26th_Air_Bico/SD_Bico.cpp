/*-----------------------
このファイルの役割：BicoでのSD用関数

------------------------*/

#include "SD_Bico.h"

//ピン配置定義ファイルを読み込む
#include "Bico_config.h"

#include "TORICA_parameters.h"


TORICA_SD SD; //引数なしでインスタンス化

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
    SD.begin(SD_CS);
}


//とりあえず20Hz書き込みで様子見
void flashSD(int flash_mode){
    memset(SD_BUF, 0, sizeof(SD_BUF)); //SD_BUFを0で初期化

    switch (flash_mode){
        case 0: {
            sprintf(SD_BUF, "%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,",  //10個
            time_ms, takeoff, speed_level, data_air_bno_accx_mss, data_air_bno_accy_mss,
            data_air_bno_accz_mss, data_air_bno_qw, data_air_bno_qx, data_air_bno_qy, data_air_bno_qz);
            SD.add_str(SD_BUF);
            SD.flash();
            break;
        }
        case 1: {
             sprintf(SD_BUF, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,", //10個
             data_air_bno_roll, data_air_bno_pitch, data_air_bno_yaw, data_air_bno_cal_system,
             data_air_bno_cal_gyro, data_air_bno_cal_accel, data_air_bno_cal_mag, data_air_bmp_pressure_hPa,
             data_air_bmp_temperature_deg, data_air_bmp_altitude_m);
             SD.add_str(SD_BUF);
             SD.flash();
             break;
        }
        case 2: {
            sprintf(SD_BUF, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,",  //10個
            data_air_gps_hour, data_air_gps_minute, data_air_gps_second, data_air_gps_centisecond,
            data_air_gps_latitude_deg, data_air_gps_longitude_deg, data_air_gps_altitude_m,
            data_air_gps_groundspeed_ms, data_air_sdp_differentialPressure_Pa, data_air_sdp_airspeed_ms);
            SD.add_str(SD_BUF);
            SD.flash();
            break;
        }
         case 3: {
            sprintf(SD_BUF, "%.2f,%.2f,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",   //10個
            data_air_AoA_angle_deg, data_air_AoS_angle_deg, data_ics_angle, data_under_bmp_pressure_hPa,
            data_under_bmp_temperature_deg, data_under_bmp_altitude_m, data_under_urm_altitude_m,
            data_under_tsd20_altitude_m, estimated_altitude_lake_m, data_altitude_bmp_urm_offset_m);
            SD.add_str(SD_BUF);
            SD.flash();
            break;
    }
    default: {
        Serial.println("Invailed argument");
    }
    }
}



//100Hz書き込み仕様．関数1回の実行につきSD書き込み1回．
//ただSD_BUFのサイズを大きくしないとだめだと思う

/*
void flashSD(){
    memset(SD_BUF, 0, sizeof(SD_BUF)); //SD_BUFを0で初期化

    //40個一気に書きこむ
    sprintf(SD_BUF, "%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", time_ms, takeoff, speed_level, data_air_bno_accx_mss, data_air_bno_accy_mss,data_air_bno_accz_mss, data_air_bno_qw, data_air_bno_qx, data_air_bno_qy, data_air_bno_qz,data_air_bno_roll, data_air_bno_pitch, data_air_bno_yaw, data_air_bno_cal_system,data_air_bno_cal_gyro, data_air_bno_cal_accel, data_air_bno_cal_mag, data_air_bmp_pressure_hPa,data_air_bmp_temperature_deg, data_air_bmp_altitude_m, data_air_gps_hour, data_air_gps_minute, data_air_gps_second, data_air_gps_centisecond,data_air_gps_latitude_deg, data_air_gps_longitude_deg, data_air_gps_altitude_m,data_air_gps_groundspeed_ms, data_air_sdp_differentialPressure_Pa, data_air_sdp_airspeed_ms,data_air_AoA_angle_deg, data_air_AoS_angle_deg, data_ics_angle, data_under_bmp_pressure_hPa,data_under_bmp_temperature_deg, data_under_bmp_altitude_m, data_under_urm_altitude_m,data_under_tsd20_altitude_m, estimated_altitude_lake_m, data_altitude_bmp_urm_offset_m);

    SD.add_str(SD_BUF);
    SD.flash();
}

*/

//100Hz保存v2
//SD書き込みを複数回に分けるタイプ．ただSDの書き込み速度が遅いとうまくいかないと思う


/*
void flashSD(){
    memset(SD_BUF, 0, sizeof(SD_BUF)); //SD_BUFを0で初期化

    sprintf(SD_BUF, "%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,",  //10個
        time_ms, takeoff, speed_level, data_air_bno_accx_mss, data_air_bno_accy_mss,data_air_bno_accz_mss, data_air_bno_qw, data_air_bno_qx, data_air_bno_qy, data_air_bno_qz);
    SD.add_str(SD_BUF);
    SD.flash();

    sprintf(SD_BUF, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,", //10個
        data_air_bno_roll, data_air_bno_pitch, data_air_bno_yaw, data_air_bno_cal_system,
        data_air_bno_cal_gyro, data_air_bno_cal_accel, data_air_bno_cal_mag, data_air_bmp_pressure_hPa, data_air_bmp_temperature_deg, data_air_bmp_altitude_m);
    SD.add_str(SD_BUF);
    SD.flash();
    
    sprintf(SD_BUF, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,",  //10個
        data_air_gps_hour, data_air_gps_minute, data_air_gps_second, data_air_gps_centisecond,data_air_gps_latitude_deg, data_air_gps_longitude_deg, data_air_gps_altitude_m,
        data_air_gps_groundspeed_ms, data_air_sdp_differentialPressure_Pa, data_air_sdp_airspeed_ms);
    SD.add_str(SD_BUF);
    SD.flash();
    
    sprintf(SD_BUF, "%.2f,%.2f,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",   //10
        data_air_AoA_angle_deg, data_air_AoS_angle_deg, data_ics_angle, data_under_bmp_pressure_hPa,data_under_bmp_temperature_deg, data_under_bmp_altitude_m, data_under_urm_altitude_m,data_under_tsd20_altitude_m, estimated_altitude_lake_m, data_altitude_bmp_urm_offset_m);
    SD.add_str(SD_BUF);
    SD.flash();

}

*/
