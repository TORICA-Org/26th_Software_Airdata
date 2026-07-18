# データ受信・パース・展開・ログ保存フロー

本ドキュメントでは、Bico基板（Main Controller）から UART (`Serial1`) で受信したテレメトリ文字列が、どのようにパースされ、`LogData` 構造体に格納・展開され、SDカードへ保存されるかの詳細なデータパイプラインを解説します。

---

## 1. データ処理パイプライン関数フローチャート

以下の図は、Core 1 での生データ受信・保存から、Core 0 での文字列パース・グローバル変数展開に至る関数の呼び出し関係とデータの流れを示しています。

```mermaid
graph TD
    subgraph Bico_Side ["送信元: Bico基板 (460,800 bps / 8E1 / カンマ区切り文字列)"]
        UART_Stream["生データパケット (4回分割・合計54項目)"]
    end

    subgraph Core1_Pipeline ["Core 1 : 受信＆ストレージ保存パイプライン"]
        L_UART["TORICA_UART::listenUART()<br>(末尾 '\n' を受信完了判定とし '\0' 置換)"]
        Copy_SD["snprintf(txData.text, '...\\n', Bico_UART.buff)<br>改行を復元してバッファ格納"]
        Push_SD_Queue["xQueueSend(sdQueue, txData)<br>キューへの即座転送"]
        
        Write_SD_Func["writeBufToSD(char* buffer)<br>TORICA_SD::add_str() で内部バッファ追記"]
        Flash_SD_Func["writeSD()<br>TORICA_SD::flash() で物理書き出し<br>(20Hz周期 / 50ms毎)"]
    end

    subgraph Core0_Pipeline ["Core 0 : パース＆構造体展開パイプライン"]
        Pop_UART_Queue["xQueueReceive(uartQueue, rxData)<br>(1秒に1回の間隔で取得)"]
        Parse_Buf["Bico_UART.parseBuffer(rxData.text)<br>カンマ区切りの文字列を float配列へ変換"]
        Check_Num{"parsed_num == BICO_DATA_NUM<br>(54個すべて揃っているか？)"}
        
        Extract_Log["extractLogData(54)<br>データの妥当性確認と展開開始"]
        Convert_Log["convertArrayToLogData(Bico_UART.UART_data)<br>float配列 [0]～[53] を struct LogData の各メンバへマッピング"]
        Apply_Globals["applyLogDataToGlobals(LogData& data)<br>構造体から volatile グローバル変数へ一括代入"]
    end

    subgraph Global_Storage ["グローバル変数群 (parameters.cpp)"]
        G_Vars["エアデータ・IMU・高度・GPS情報<br>(volatile float/int/bool)"]
    end

    %% フロー接続
    UART_Stream --> L_UART
    L_UART --> Copy_SD
    Copy_SD --> Push_SD_Queue
    Push_SD_Queue ==> Write_SD_Func
    Write_SD_Func --> Flash_SD_Func

    Copy_SD -->|25回に1回<br>(4Hzサンプリング)| Pop_UART_Queue
    Pop_UART_Queue --> Parse_Buf
    Parse_Buf --> Check_Num
    Check_Num -->|Yes| Extract_Log
    Check_Num -->|No / パケット欠落| Drop["破棄 (パースエラー時は変数更新をスキップ)"]
    Extract_Log --> Convert_Log
    Convert_Log --> Apply_Globals
    Apply_Globals ==> G_Vars
```

---

## 2. 54項目のデータインデックスマッピング (`convertArrayToLogData()`)

Bico基板からは、1サイクルのデータとして合計 **54項目** がカンマ区切り文字列で送信されます。`Bico_UART.parseBuffer()` によって `float Bico_UART.UART_data[54]` に格納された後、以下のように各センサの変数へとマッピングされます。

| 受信パケット分割 | インデックス範囲 | 項目数 | 主な内容と対象センサ |
| :--- | :---: | :---: | :--- |
| **第1パケット (13項目)** | `[0] ～ [12]` | 13個 | **システム時刻・離陸判定・GPS基本情報**<br>`time_ms`, `takeoff`, `urm_is_reliable`, `gps_hour/minute/second/centisecond`, `gps_latitude/longitude/altitude`, `gps_groundspeed/heading/satellites` |
| **第2パケット (11項目)** | `[13] ～ [23]` | 11個 | **エアデータ・フィルタリング結果・気流角**<br>`filtered_bmp_alt`, `filtered_urm_alt`, `filtered_airspeed`, `air_bmp_pressure/temp/alt` (BMP390), `sdp_diffPressure/airspeed` (SDP31), `AoA_angle`, `AoS_angle`, `ics_angle` |
| **第3パケット (14項目)** | `[24] ～ [37]` | 14個 | **胴体桁電装ステータス・姿勢角 (IMU / BMP)**<br>`fslg_is_alive`, `bno_qw/qx/qy/qz` & `roll/pitch/yaw` (BNO055姿勢), `lsm_roll/pitch/yaw` (LSM6DSV16X姿勢), `fslg_bmp_pressure/temp/alt` |
| **第4パケット (16項目)** | `[38] ～ [53]` | 16個 | **IMU加速度・キャリブレーション・Under電装**<br>`fslg_bno_accx/y/z`, `fslg_lsm_accx/y/z`, `bno_cal_system/gyro/accel/mag` (キャリブレーションステータス), `under_is_alive`, `under_bmp_pressure/temp/alt`, `under_urm_alt`, `under_tsd20_alt` |

---

## 3. SDカード CSV ヘッダー構造 (`flashHeader()`)

`setup()` 内で実行される `flashHeader()` (`SD_Air_xiao.cpp`) では、上記4パケット・計54項目に対応する CSV ヘッダー文字列が、`snprintf` と `sd.add_str()` によって microSD カードへ書き込まれます。

- ヘッダー書き込みの際は、マイコンやSPIバスのバッファ溢れを防ぐため、`case 0` 〜 `case 3` の4回に分けて書き込みと `sd.flash()` を実行し、間に `delayMicroseconds(10)` を挟む安全設計となっています。
- コア1 (`processCore1_ListenUART()`) から送られてきた文字列行にはすでに全項目がカンマ区切りで並んでいるため、SDカード内には **「完全なCSV形式のフライトデータログ」** が時系列で記録され続けます。
