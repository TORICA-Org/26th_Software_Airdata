#include "Teseo_LIV3FL.h"
#include <Arduino.h>
#include <stdint.h>
#include <TinyGPSPlus.h>
#include "parameters.h"

/*
コマンド文字列データフォーマット
RAM上の設定を設定：$PSTMSETPAR,<ConfigBlock><ID>,<param_value>[,<mode>]*<checksum><cr><lf>
例) Baudrateを921600bpsに変更
$PSTMSETPAR,1102,0xD*15
ConfigBlock -> 1, ID -> 102, param_value -> 0xD, checksum->15
*/

// TinyGPSPlusを使った処理
TinyGPSPlus gps;











/*------ 以下Teseo用カスタム関数 -------*/

Teseo_LIV3FL::Teseo_LIV3FL(Stream& serialPort) : serial(serialPort) {}

//チェックサム計算用関数
String Teseo_LIV3FL::calc_checksum(const char* cmd){
/*コマンド文字列のEX-ORを計算する．(ただし先頭の$は計算に含めない)*/
    uint8_t checksum = 0;
    for (int i=0; cmd[i]!= '\0'; i++){
        //コマンド文字列(cmd)の終端になるまで計算
        checksum ^= cmd[i];
    }
    //計算結果を文字列に変換
    String checksum_str = String(checksum, HEX);
    checksum_str.toUpperCase();
    //チェックサムが1桁の場合は先頭に0を追加して2桁に
    if (checksum_str.length() == 1) {
        checksum_str = "0" + checksum_str;
    }
    return checksum_str;
}

void Teseo_LIV3FL::setBaudrate(int bps){
    // 送信するコマンド文字列を格納するポインタ
    const char* cmd_str = nullptr;

    //不格好だけど処理の流れをわかりやすくするためにswitch文で分岐
    switch (bps) {
        case 300:
            //300bpsのとき
            cmd_str = "PSTMSETPAR,1102,0x0";
            break;
        
        case 600:
            //600bpsのとき
            cmd_str = "PSTMSETPAR,1102,0x1";
            break;
        
        case 1200:
            //1200bpsのとき
            cmd_str = "PSTMSETPAR,1102,0x2";
            break;
        
        case 2400:
            //2400bpsのとき
            cmd_str = "PSTMSETPAR,1102,0x3";
            break;
        
        case 4800:
            //4800bpsのとき
            cmd_str = "PSTMSETPAR,1102,0x4";
            break;
        
        case 9600:
            //9600bpsのとき
            cmd_str = "PSTMSETPAR,1102,0x5";
            break;
        
        case 14400:
            //14400bpsのとき
            cmd_str = "PSTMSETPAR,1102,0x6";
            break;
        
        case 19200:
            //19200bpsのとき
            cmd_str = "PSTMSETPAR,1102,0x7";
            break;
        
        case 38400:
            //38400bpsのとき
            cmd_str = "PSTMSETPAR,1102,0x8";
            break;
        
        case 57600:
            //57600bpsのとき
            cmd_str = "PSTMSETPAR,1102,0x9";
            break;
        
        case 115200:
            //115200bpsのとき
            cmd_str = "PSTMSETPAR,1102,0xA";
            break;
        
        case 230400:
            //230400bpsのとき
            cmd_str = "PSTMSETPAR,1102,0xB";
            break;
        
        case 460800:
            //460800bpsのとき
            cmd_str = "PSTMSETPAR,1102,0xC";
            break;

        case 921600:
            //921600bps
            cmd_str = "PSTMSETPAR,1102,0xD";
            break;

        default:
            // 対応していないボーレートの場合は何もしない
            Serial.println("Unsupported baudrate."); //USB経由でエラー出力
            return; 
    }

    // チェックサムの計算
    String cs = calc_checksum(cmd_str);

    // コマンドの送信 ($コマンド文字列*チェックサム\r\n)
    serial.print("$");
    serial.print(cmd_str);
    serial.print("*");
    serial.println(cs); //文字列の末尾なので\r\n付きで送信

    Serial.print("$");
    Serial.print(cmd_str);
    Serial.print("*");
    Serial.println(cs); //USB経由で出力
}

//更新速度変更コマンド
void Teseo_LIV3FL::setfixrate(int rate_Hz){
    float rate_s = 1.0 / rate_Hz; //Hzから秒に変換
    if (rate_s < 0.1) {
        Serial.println("Unsupported update rate."); //10Hz以上(0.1s以下)は非対応
        return;
    }

    //送信するコマンド文字列をStringで組み立てる
    String cmd_str = "PSTMSETPAR,1101," + String(rate_s, 2); //小数点以下2桁までの文字列に変換
    String cs = calc_checksum(cmd_str.c_str());

    serial.print("$");
    serial.print(cmd_str);
    serial.print("*");
    serial.println(cs);

}

//設定保存コマンド $PSTMSAVEPAR*58
void Teseo_LIV3FL::save(){
    String cs = calc_checksum("PSTMSAVEPAR");
    serial.print("$PSTMSAVEPAR*");
    serial.println(cs);
}

//再起動コマンドの送信 $PSTMSRR*49
void Teseo_LIV3FL::reboot(){
    // チェックサム計算
    String cs = calc_checksum("PSTMSRR");
    serial.print("$PSTMSRR*");
    serial.println(cs);
}

//cold start $PSTMCOLD*1E
void Teseo_LIV3FL::coldstart(){
    String cs = calc_checksum("PSTMCOLD");
    serial.print("$PSTMCOLD*");
    serial.println(cs);
}