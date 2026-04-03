#include "speaker.h"

void speaker_init(){
    
}

void speaker(){
    static int sound_freq = 440; //サウンド周波数
    static int spk_flag = 0;
    static uint32_t speaker_last_change_time = millis();
    static uint32_t sound_duration = 100; //音が出ている時間


}