# データ受信・物理演算・送信パイプラインフロー

`26th_Air_Bico` は、自基板搭載の 4 基の I2C センサー、1 系統の GPS、および 3 つの外部基板（Under, 胴体桁, ICS）から集まる膨大な生データを統合・解析し、飛行に必須のエアデータ計算（高度・対気速度・離陸判定）を完了させた上で、地上親機（Xiao ESP32 S3）とストレージへ配信するパイプラインを構築しています。

---

## 1. データ収集・計算・出力の統合パイプラインフローチャート

```mermaid
flowchart TD
    subgraph Core0_Input ["Core 0 : 自基板センサー＆GPS取得 (100Hz)"]
        S_BMP["BMP3XX_init / read_bmp()<br>気圧・温度 (Wire1, 0x76)"]
        S_AS["AS5600_init / read_AS5600()<br>迎え角 AoA・横滑り角 AoS (Wire)"]
        S_SDP["SDP31_init / read_SDP()<br>差圧 Pa (Wire, 0x23)"]
        S_GPS["GPSHelper / read_gps()<br>GPS緯度経度・速度・衛星数 (10Hz)"]
        
        S_BMP --> U_BMP["data_air_bmp_pressure_hPa / temp / alt"]
        S_AS --> U_AS["data_air_AoA_angle_deg / AoS_angle_deg"]
        S_SDP --> U_SDP["data_air_sdp_differentialPressure_Pa"]
        S_GPS --> U_GPS["data_air_gps_latitude_deg 等 10項目"]
    end

    subgraph Core1_Input ["Core 1 : 外部基板 UART テレメトリ受信 (100Hz)"]
        R_Under["receiveUnderLog()<br>Under_UART.readUART() 5項目受信<br>(気圧, 温度, 高度, URM超音波, TSD20 LiDAR)"]
        R_Fslg["receiveFslgLog()<br>Fslg_UART.readUART() 23項目受信<br>(BNO055姿勢/加速度/キャリブ, BMP, LSM姿勢/加速度)"]
        R_ICS["receiveIcsAngle()<br>TORICA_ICS::read_Angle() 1項目受信<br>(操舵角データ)"]
    end

    subgraph Globals ["グローバル共有データストレージ (parameters.cpp)"]
        G_Box["54項目の volatile グローバル変数群<br>& struct SharedSensorData (Core間渡し)"]
    end

    subgraph Core1_Calc ["Core 1 : エアデータ物理演算＆フィルタリング層"]
        C_Alt["calculate_altitude(気圧, 超音波...)<br>1. 3基板(Air/Under/Fslg)の気圧高度計算<br>2. プラットフォーム補正後の中央値採用<br>3. 超音波(URM)フィルタリングと信頼性判定"]
        C_Spd["calculate_airspeed(差圧, 気圧, 温度...)<br>1. 対気速度公式で真対気速度を導出<br>2. correct_airspeed() で二次式補正適用<br>3. 移動平均フィルタに反映"]
        C_TO["is_takeoff()<br>TSD20 LiDAR フィルタ済み高度 > 3.0m で<br>takeoff = true に確定"]
    end

    subgraph Core1_Output ["Core 1 : パケット生成＆DMA送信パイプライン"]
        F_Log["transmitLog(trans_mode 0〜3)<br>sprintf() による CSV 文字列フォーマット化"]
        F_Fslg["transmitLog_for_fslg(trans_mode 0〜2)<br>胴体桁専用 29項目 CSV フォーマット化"]

        DMA_Tx["DMA チャネル非同期送信<br>Serial_ESP (Xiao) へ 460,800bps 転送"]
        Sync_Tx["同期シリアル送信<br>Serial_Under へそのまま SD ログ用書き込み"]
    end

    %% --- フロー接続 ---
    U_BMP & U_AS & U_SDP & U_GPS ===> G_Box
    R_Under & R_Fslg & R_ICS ===> G_Box

    G_Box ===> C_Alt & C_Spd & C_TO
    C_Alt & C_Spd & C_TO ==> G_Box

    G_Box ===> F_Log & F_Fslg
    F_Log --> DMA_Tx & Sync_Tx
    F_Fslg -->|"Serial_fslg.print()"| Board_Fslg_Tx["Serial_fslg (胴体桁基板)"]
```

---

## 2. 54項目データパケットのインデックス構成 (`struct LogData` / 4分割送信)

`26th_Air_Bico` は、毎回の送信でマイコンや UART DMA のバッファをあふれさせず、かつ各パケットのデータ長を均等化するため、54 項目のエアデータを **4 つの送信モード (`trans_mode = 0 〜 3`)** に分けて `Serial_ESP` および `Serial_Under` へ送信します。

| 送信モード (`trans_mode`) | 項目数 | データ内容とインデックス範囲 |
| :---: | :---: | :--- |
| **`case 0`** (計13項目) | 13個 | **システム時刻・離陸判定・GPS基本情報**<br>`time_ms`, `takeoff`, `urm_is_reliable`, `data_air_gps_hour/minute/second/centisecond`, `data_air_gps_latitude/longitude/altitude`, `data_air_gps_groundspeed/heading/satellites` |
| **`case 1`** (計11項目) | 11個 | **フィルタ済みエアデータ・気圧・気流角・操舵角**<br>`filtered_bmp_altitude_m`, `filtered_urm_altitude_m`, `filtered_airspeed_ms`, `data_air_bmp_pressure/temperature/altitude`, `data_air_sdp_differentialPressure/airspeed`, `data_air_AoA/AoS_angle_deg`, `data_ics_angle` |
| **`case 2`** (計14項目) | 14個 | **胴体桁生存ステータス・BNO055/LSM6DSV16X 姿勢角・気圧**<br>`fslg_is_alive`, `data_fslg_bno_qw/qx/qy/qz`, `roll/pitch/yaw`, `data_fslg_lsm_roll/pitch/yaw`, `data_fslg_bmp_pressure/temperature/altitude` |
| **`case 3`** (計16項目) | 16個 | **加速度・IMU キャリブレーション・機体下基板データ**<br>`data_fslg_bno_accx/accy/accz`, `lsm_accx/accy/accz`, `bno_cal_system/gyro/accel/mag`, `under_is_alive`, `data_under_bmp_pressure/temperature/altitude`, `under_urm_altitude_m`, `under_tsd20_altitude_m` |

---

## 3. CSV ヘッダー送信機構 (`transmitHeader()`) の工夫

`setup()` 内で実行される `transmitHeader()` は、上記 4 分割パケット・全 54 項目に対応するカンマ区切りの CSV 列見出し（文字列）を生成し、`Serial_ESP` (Xiao) および `Serial_Under` (SD書き込み用) に一括送信します。

### バッファ溢れ防止と時間遅延 (`delayMicroseconds(10)`)
- 54 個の変数名をカンマで繋ぐと数百バイトに達するため、一度に送信しようとすると RP2040 のハードウェア UART 送信 FIFO (デフォルト 32 バイト) が即座に満杯となり、文字欠けやパケット破損が発生します。
- これを防ぐため、`case 0` 〜 `case 3` の 4 ステップのループに分割して `sprintf` でフォーマットし、**各ステップの間で `Serial_Under.flush()` および `delayMicroseconds(10)` を挿入** することで、受信側（Xiao や Under 基板の SD 書き込み処理）が取りこぼすことなく安全に CSV ヘッダーを記録できるようにしています。
