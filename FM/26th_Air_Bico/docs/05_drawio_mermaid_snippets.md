# draw.io 貼り付け専用 Mermaid スニペット集 (`26th_Air_Bico` / RP2040)

## 1. システム全体アーキテクチャ図 (`README.md`)

```mermaid
flowchart TD
    subgraph External ["外部基板＆接続デバイス"]
        Sens_Local["自基板I2Cセンサー (Wire/Wire1)<br>・BMP390 (気圧/温度)<br>・SDP31 (差圧)<br>・AS5600 × 2 (迎え角 AoA / 横滑り角 AoS)"]
        Sens_GPS["GPSモジュール (Serial_GPS)<br>・10Hz / ボーレート依存なし受信"]
        Board_Under["Under基板 (Serial_Under / SerialPIO)<br>・気圧, 超音波(URM), LiDAR(TSD20)"]
        Board_Fslg["胴体桁基板 / Fslg (Serial_fslg / SerialPIO)<br>・BNO055(9軸), LSM6DSV16X(6軸), BMP390"]
        Board_ICS["ICS操舵基板 (Serial_ICS / Serial1)<br>・操舵角データ (115,200 bps 8E1)"]
        Board_Xiao["Xiao ESP32 S3基板 (Serial_ESP / Serial2)<br>・地上モニタリング＆SDレコーダー親機<br>・(460,800 bps 8E1 / DMA高速転送)"]
    end

    subgraph Core0 ["Core 0 : センサー計測＆GPS専用タスク (100Hz / 10ms周期タイマー割り込み)"]
        Task_C0["setup() / loop()<br>core0_timer_callback (-10ms)"]
        Read_Sens["update_air_bmp() / update_air_AS5600() / update_air_SDP()<br>自基板センサー取得"]
        Read_GPS["update_air_gps()<br>10サイクル毎 (10Hz) のGPSパース"]
        WD_Check["watchdog_update()<br>Core 1 生存監視とハードウェアWDTクリア (2000ms)"]
    end

    subgraph Sync_Layer ["マルチコア同期・共有メモリ層 (pico/mutex & pico/multicore)"]
        Mutex_Sensor["mutex_t sensor_mutex<br>共有構造体 SharedSensorData の保護"]
        FIFO_Signal["multicore_fifo<br>Core 0 -> Core 1 更新完了シグナル (値: 1)"]
    end

    subgraph Core1 ["Core 1 : UART通信＆物理計算専用タスク (100Hz / 10ms周期タイマー割り込み)"]
        Task_C1["setup1() / loop1()<br>core1_timer_callback (-10ms)"]
        Read_UARTs["receiveUnderLog() / receiveFslgLog() / receiveIcsAngle()<br>他基板からのテレメトリ受信＆生存確認"]
        Calc_Physics["calculate_altitude() / calculate_airspeed() / is_takeoff()<br>気圧高度・対気速度・LiDAR離陸判定計算"]
        Cmd_Handler["handleEspSignal()<br>Xiao からのコマンド検知 (RESET/TAKEOFF等)"]
        Tx_Logger["transmitLog() / transmitLog_for_fslg()<br>4分割時分割パケット DMA送信 (計54項目)"]
    end

    subgraph Globals ["グローバル共有状態 (parameters.h / .cpp)"]
        Global_Vars["54項目のフライトログ・エアデータ変数<br>・volatile float / uint32_t / bool<br>・time_ms, takeoff, urm_is_reliable 等"]
    end

    Sens_Local -->|"I2C読み出し"| Read_Sens
    Sens_GPS -->|"UARTパース"| Read_GPS
    Read_Sens -->|"ロック取得してコピー"| Mutex_Sensor
    Read_GPS -->|"グローバル直接更新"| Global_Vars
    Task_C0 -->|"シグナルプッシュ"| FIFO_Signal

    FIFO_Signal -->|"ポップ検知"| Task_C1
    Mutex_Sensor -->|"ロック取得してローカル変数へコピー"| Calc_Physics

    Board_Under -->|"UART受信 460.8kbps"| Read_UARTs
    Board_Fslg -->|"UART受信 460.8kbps"| Read_UARTs
    Board_ICS -->|"UART受信 115.2kbps"| Read_UARTs
    Read_UARTs -->|"グローバル更新"| Global_Vars

    Calc_Physics -->|"計算結果・離陸フラグ格納"| Global_Vars
    Global_Vars -->|"送信データ生成"| Tx_Logger

    Tx_Logger -->|"DMA / 同期高速送信 (54項目)"| Board_Xiao
    Tx_Logger -->|"SD書き込み用コピー送信"| Board_Under
    Tx_Logger -->|"胴体桁用データ送信 (29項目)"| Board_Fslg

    Board_Xiao -->|"コマンドシグナル送信"| Cmd_Handler
    Cmd_Handler -->|"コマンド転送 / takeoff反転"| Board_Under & Board_Fslg & Global_Vars
    Task_C1 -->|"core1_alive = true"| WD_Check
```

## 2. インクルード関係グラフ (`01_file_relationships.md`)

```mermaid
flowchart TD
    subgraph Core_Headers ["共通基盤層 (ヘッダーと設定)"]
        Config_H["Bico_config.h<br>(ピン配置・ペリフェラル設定)"]
        Params_H["parameters.h<br>(volatile 変数・LogData 構造体宣言)"]
    end

    subgraph Sensor_Drivers ["センサーアクセスドライバ層"]
        AS_H["AS5600.h"]
        AS_C["AS5600.cpp"]
        BMP_H["BMP3xx.h"]
        BMP_C["BMP3xx.cpp"]
        SDP_H["SDP31.h"]
        SDP_C["SDP31.cpp"]
        GPS_H["GPSHelper.h"]
        GPS_C["GPSHelper.cpp"]
    end

    subgraph Logic_Modules ["物理計算＆通信統括モジュール層"]
        SR_H["sensor_reader.h"]
        SR_C["sensor_reader.cpp"]
        Alt_H["calculate_altitude.h"]
        Alt_C["calculate_altitude.cpp"]
        Spd_H["calculate_airspeed.h"]
        Spd_C["calculate_airspeed.cpp"]
        UART_H["UARTHelper_Bico.h"]
        UART_C["UARTHelper_Bico.cpp"]
    end

    subgraph Main_Entry ["メインエントリポイント"]
        Main_INO["26th_Air_Bico.ino"]
    end

    AS_C --> AS_H
    BMP_C --> BMP_H
    SDP_C --> SDP_H
    GPS_C --> GPS_H

    SR_H --> Params_H
    SR_C --> SR_H
    SR_C --> Params_H
    SR_C --> BMP_H
    SR_C --> AS_H
    SR_C --> SDP_H
    SR_C --> GPS_H

    Alt_H --> Params_H
    Alt_C --> Alt_H
    Alt_C --> Params_H

    Spd_H --> Params_H
    Spd_C --> Spd_H
    Spd_C --> Params_H

    UART_H --> Params_H
    UART_C --> UART_H
    UART_C --> Config_H

    Main_INO --> Params_H
    Main_INO --> Config_H
    Main_INO --> Alt_H
    Main_INO --> Spd_H
    Main_INO --> AS_H
    Main_INO --> BMP_H
    Main_INO --> SDP_H
    Main_INO --> UART_H
    Main_INO --> GPS_H
    Main_INO --> SR_H
```

## 3. スレッドセーフデータ共有設計図 (`01_file_relationships.md`)

```mermaid
flowchart LR
    subgraph Core0_Task ["Core 0 タスク (I2C / GPS 読取)"]
        C0_Read["自基板センサー値の更新<br>data_air_bmp_pressure_hPa 等"]
        C0_Lock["mutex_enter_blocking(&sensor_mutex)"]
        C0_Copy["shared_sensor_data にスナップショットをコピー"]
        C0_Unlock["mutex_exit(&sensor_mutex)"]
        C0_Push["multicore_fifo_push_blocking(1)<br>シグナル送信"]
    end

    subgraph Shared_Memory ["共有メモリ保護領域"]
        Mutex["mutex_t sensor_mutex<br>(RP2040 ハードウェア Mutex)"]
        Shared_Struct["struct SharedSensorData<br>(気圧・温度・超音波・差圧スナップショット)"]
        FIFO["multicore_fifo<br>(コア間通信 FIFO レジスタ)"]
    end

    subgraph Core1_Task ["Core 1 タスク (UART / 物理計算)"]
        C1_Check{"multicore_fifo_rvalid() ?"}
        C1_Pop["multicore_fifo_pop_blocking()"]
        C1_Lock["mutex_enter_blocking(&sensor_mutex)"]
        C1_Copy["local_air_press 等のローカル変数へ安全コピー"]
        C1_Unlock["mutex_exit(&sensor_mutex)"]
        C1_Calc["calculate_altitude() / calculate_airspeed()<br>ローカル変数のみを引数に物理計算を実行"]
    end

    C0_Read --> C0_Lock --> C0_Copy --> C0_Unlock --> C0_Push
    C0_Copy ==> Shared_Struct
    C0_Lock -.-> Mutex
    C0_Unlock -.-> Mutex
    C0_Push ==> FIFO

    FIFO ==> C1_Check
    C1_Check -->|"Yes"| C1_Pop --> C1_Lock --> C1_Copy --> C1_Unlock --> C1_Calc
    C1_Check -->|"No (シグナルなし)"| C1_Calc
    C1_Copy <== Shared_Struct
    C1_Lock -.-> Mutex
    C1_Unlock -.-> Mutex
```

## 4. 初期化シーケンス図 (`02_core_tasks_flowchart.md`)

```mermaid
sequenceDiagram
    autonumber
    participant Core0 as Core 0 : setup() [main]
    participant Core1 as Core 1 : setup1() [multicore]
    participant HW as ペリフェラル / ピン
    participant UART as UARTHelper / Serial
    participant Sens as I2Cセンサー (SDP, AS5600, BMP)
    participant Timer as ハードウェアタイマー (100Hz)
    participant WDT as Watchdog Timer

    Note over Core0,Core1: システム起動（電源投入またはリセット）
    Core0->>HW: mutex_init(&sensor_mutex) & LED ピン出力設定
    Core0->>UART: Serial.begin(115200) [デバッグ用 USB-UART]
    Core0->>UART: initUART() & initUART_DMA() [ICS/Xiao/Under/Fslg 通信開始]
    Core0->>UART: initGPS() [Serial_GPS 開始]
    Core0->>UART: transmitHeader() [SDカード用 CSV ヘッダー送信]

    Core0->>HW: Wire / Wire1 I2C ピン指定と 400kHz クロック設定
    Core0->>Sens: SDP31_init(&Wire, 0x23)
    Core0->>Sens: AS5600_init() [迎え角・横滑り角センサー 2基]
    Core0->>Sens: BMP3XX_init(&Wire1, 0x76)

    Core0->>WDT: watchdog_enable(2000, 1) [2秒タイムアウト設定]
    Core0->>Timer: add_repeating_timer_ms(-10, core0_timer_callback, &core0_timer)

    Note over Core1: Core 1 の個別初期化
    Core1->>Timer: add_repeating_timer_ms(-10, core1_timer_callback, &core1_timer)
    Note over Core0,Core1: 双方のコアでタイマー割り込みによる 100Hz 定周期ループへ移行
```

## 5. Core 0 vs Core 1 タスクループフローチャート (`02_core_tasks_flowchart.md`)

```mermaid
flowchart TD
    subgraph Core0_Loop ["Core 0 ループ : loop() 【実行周期: 100Hz / 10ms】"]
        C0_Start(("Core 0 ループ開始")) --> C0_CheckFlag{"core0_timer_triggered<br>== true ?"}
        C0_CheckFlag -->|No| C0_Idle["待機 (次のタイマー割り込みまで)"]
        C0_CheckFlag -->|Yes| C0_Clear["core0_timer_triggered = false<br>LED全点灯 & time_ms 記録"]

        C0_Clear --> C0_Sensors["センサー読み取り<br>1. update_air_bmp() (気圧/温度)<br>2. update_air_AS5600() (AoA/AoS)<br>3. update_air_SDP() (差圧)"]
        
        C0_Sensors --> C0_CheckGPS{"GPS カウンタ >= 10 ?<br>(10回毎 = 10Hz)"}
        C0_CheckGPS -->|Yes| C0_GPS["update_air_gps()<br>GPS緯度経度・速度・高度更新"]
        C0_CheckGPS -->|No| C0_Sync
        C0_GPS --> C0_Sync

        C0_Sync["排他ロック＆シグナル送信<br>1. mutex_enter_blocking(&sensor_mutex)<br>2. shared_sensor_data へスナップショット格納<br>3. mutex_exit(&sensor_mutex)<br>4. multicore_fifo_push_blocking(1)"]

        C0_Sync --> C0_LED_Low["LED 全消灯"]
        C0_LED_Low --> C0_CheckWDT{"core1_alive == true ?<br>(Core 1 が生存しているか)"}
        
        C0_CheckWDT -->|Yes| C0_WDT_Update["watchdog_update() で WDT クリア<br>core1_alive = false へリセット"]
        C0_CheckWDT -->|No| C0_End(("処理完了"))
        C0_WDT_Update --> C0_End
    end

    subgraph FIFO_Layer ["ハードウェア FIFO ＆ Mutex"]
        FIFO_Box["multicore_fifo レジスタ"]
        Mutex_Box["sensor_mutex"]
    end

    subgraph Core1_Loop ["Core 1 ループ : loop1() 【実行周期: 100Hz / 10ms】"]
        C1_Start(("Core 1 ループ開始")) --> C1_CheckFlag{"core1_timer_triggered<br>== true ?"}
        C1_CheckFlag -->|No| C1_Idle["待機 (次のタイマー割り込みまで)"]
        C1_CheckFlag -->|Yes| C1_Clear["core1_timer_triggered = false"]

        C1_Clear --> C1_Receive["他基板テレメトリ受信<br>1. receiveUnderLog() (機体下)<br>2. receiveFslgLog() (胴体桁)<br>3. receiveIcsAngle() (ICS)<br>4. handleEspSignal() (コマンド確認)"]

        C1_Receive --> C1_CheckFIFO{"multicore_fifo_rvalid() ?<br>(Core 0 からの更新通知あり)"}
        C1_CheckFIFO -->|Yes| C1_PopCopy["FIFO ポップ & Mutex ロック<br>shared_sensor_data からローカル変数へコピー"]
        C1_CheckFIFO -->|No| C1_NoCopy["前回のローカル変数を維持"]

        C1_PopCopy --> C1_Calc
        C1_NoCopy --> C1_Calc

        C1_Calc["エアデータ物理演算<br>1. calculate_altitude(ローカル変数...)<br>2. calculate_airspeed(ローカル変数...)<br>3. is_takeoff() (LiDAR高度による離陸判定)"]

        C1_Calc --> C1_Transmit["時分割テレメトリ DMA 送信<br>1. transmitLog(transmit_count 0〜3) -> Xiao/Under<br>2. transmitLog_for_fslg(0〜2) -> 胴体桁"]

        C1_Transmit --> C1_Alive["core1_alive = true<br>生存フラグを立てる"]
        C1_Alive --> C1_End(("処理完了"))
    end

    C0_Sync ===> FIFO_Box
    C0_Sync -.-> Mutex_Box
    FIFO_Box ===> C1_CheckFIFO
    Mutex_Box -.-> C1_PopCopy
    C1_Alive -.-> C0_CheckWDT
```

## 6. データパイプライン関数呼び出しフロー図 (`03_data_pipeline_flowchart.md`)

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

    U_BMP & U_AS & U_SDP & U_GPS ===> G_Box
    R_Under & R_Fslg & R_ICS ===> G_Box

    G_Box ===> C_Alt & C_Spd & C_TO
    C_Alt & C_Spd & C_TO ==> G_Box

    G_Box ===> F_Log & F_Fslg
    F_Log --> DMA_Tx & Sync_Tx
    F_Fslg -->|"Serial_fslg.print()"| Board_Fslg_Tx["Serial_fslg (胴体桁基板)"]
```

## 7. 時分割テレメトリ送信条件分岐図 (`04_web_command_flowchart.md`)

```mermaid
flowchart TD
    Start_Tx(("transmitLog(transmit_count) 呼び出し<br>【100Hz / 10ms周期】")) --> Check_Count{"transmit_count の値判定<br>(0 〜 3)"}

    Check_Count -->|trans_mode == 0| M0["【モード 0】 システム・GPS情報 (計13項目)<br>送信: time_ms, takeoff, urm_is_reliable, GPS時刻(時/分/秒/100分の一秒),<br>GPS緯度経度(%.7f), 気圧高度, 地表速度, 方位, 衛星数"]
    
    Check_Count -->|trans_mode == 1| M1["【モード 1】 フィルタ済みエアデータ (計11項目)<br>送信: フィルタ気圧/超音波/対気速度, 自基板BMP気圧/温度/高度,<br>SDP31差圧/対気速度, 迎え角 AoA, 横滑り角 AoS, ICS操舵角"]
    
    Check_Count -->|trans_mode == 2| M2["【モード 2】 胴体桁 IMU・気圧情報 (計14項目)<br>送信: fslg生存フラグ, BNO055クォータニオン(qw/qx/qy/qz),<br>BNO055オイラー角, LSM6DSV16Xオイラー角, 胴体桁BMP気圧/温度/高度"]
    
    Check_Count -->|trans_mode == 3| M3["【モード 3】 加速度・キャリブ・Under情報 (計16項目)<br>送信: BNO055/LSM6DSV16X加速度, BNOキャリブ値(4項目),<br>Under生存フラグ, Under BMP気圧/温度/高度, 超音波・TSD20高度"]

    M0 & M1 & M2 & M3 --> Check_DMA{"DMA 転送使用 ?<br>(#define USE_DMA)"}
    
    Check_DMA -->|Yes| DMA_Send["dma_channel_configure() で非同期 DMA 転送開始<br>Serial_ESP (Xiao) へバッファを直接送り出し"]
    Check_DMA -->|No / Sync| Sync_Send["Serial_ESP.flush() & print() による同期送信"]

    DMA_Send & Sync_Send --> Under_Send["Serial_Under.flush() & print() による同期送信<br>(Under基板の SD カード記録用)"]
    
    Under_Send --> Inc_Count["transmit_count++<br>(次回モードへ)"]
    Inc_Count --> Wrap_Check{"transmit_count > 3 ?"}
    Wrap_Check -->|Yes| Reset_Count["transmit_count = 0 にリセット"]
    Wrap_Check -->|No| End_Tx(("次の 10ms ループまで待機"))
    Reset_Count --> End_Tx
```

## 8. コマンド制御シーケンス図 (`04_web_command_flowchart.md`)

```mermaid
sequenceDiagram
    autonumber
    participant Xiao as Xiao ESP32 S3 (親機・Web受信)
    participant Core1 as Core 1 : handleEspSignal()
    participant Glob as グローバル変数
    participant Under as Under 基板 (Serial_Under)
    participant Fslg as 胴体桁 基板 (Serial_fslg)

    Xiao->>Core1: シリアルコマンド文字列送信 (460,800 bps)
    Note over Core1: ESP_UART.listenUART() で行データ受信完了検知

    alt 文字列内に "RESET" が含まれている場合
        Core1->>Under: Serial_Under.print("\nRESET\n") -> SDカードにリセット行記録
        Core1->>Fslg: Serial_fslg.print("RESET") -> 胴体桁のログや変数リセット
    else 文字列内に "SPK_EN" が含まれている場合 (スピーカーON)
        Core1->>Fslg: Serial_fslg.print("SPK_EN") -> 胴体桁ブザー・音声出力開始
    else 文字列内に "SPK_DIS" が含まれている場合 (スピーカーOFF)
        Core1->>Fslg: Serial_fslg.print("SPK_DIS") -> 胴体桁ブザー停止
    else 文字列内に "CHG_TO" が含まれている場合 (離陸判定反転)
        Core1->>Fslg: Serial_fslg.print("CHG_TO") -> スピーカー・表示用通知
        Core1->>Glob: takeoff = !takeoff (離陸判定フラグを強制反転)
    end
```

## 9. 電圧・電流計測及び物理計算回路図 (`04_web_command_flowchart.md`)

```mermaid
flowchart LR
    subgraph Alt_Calc ["気圧高度物理公式 ＆ 中央値選択回路 (calculate_altitude.cpp)"]
        Press["各基板気圧 P [hPa]<br>＆ 気温 T [℃]"] -->|"国際標準大気公式計算"| Eq_Alt["h = ((1013.25 / P)^(1 / 5.257) - 1)<br>× (T + 273.15) / 0.0065"]
        Eq_Alt -->|"プラットフォーム平均補正"| Offset["差分 = (現在高度) - (プラホ平均)<br>+ プラットフォーム定数 (10.6m)"]
        Offset -->|"QuickStats.median(3)"| Median["Air, Under, Fslg の 3系統<br>中央値を選択 -> filtered_bmp_altitude_m"]
    end

    subgraph Spd_Calc ["対気速度物理公式 ＆ 二次式補正回路 (calculate_airspeed.cpp)"]
        SDP["SDP31 差圧 ΔP [Pa]"] --> Eq_Spd["真対気速度導出公式<br>v = √| 2ΔP × (T / P) × 287.026 |"]
        Press & Eq_Alt -.-> Eq_Spd
        Eq_Spd -->|"二次式校正 correct_airspeed()"| Eq_Calib["v_calib = A×v^2 + B×v + C<br>(現在は係数 0.0 で生値通過)"]
        Eq_Calib -->|"5タップ移動平均"| MoveAve["filtered_airspeed_ms へ格納"]
    end

    subgraph TO_Calc ["LiDAR 高度離陸判定回路 (is_takeoff() in calculate_altitude.cpp)"]
        LiDAR["TSD20 LiDAR 高度 [m]"] -->|"5タップ移動平均"| LiDAR_Ave["filtered_tsd20_altitude_m"]
        LiDAR_Ave -->|"判定閾値 > 3.0m ?"| Check_TO{"高度 > 3.0m ?<br>(かつ takeoff == false)"}
        Check_TO -->|Yes| Set_TO["takeoff = true に確定<br>(以降はスマホ Web コマンドでしか false に戻せない)"]
    end
```

## 10. 4層レイヤー構造・関数ヒエラルキーと処理関係図 (`06_layer_hierarchy_flowchart.md`)

```mermaid
flowchart TD
    subgraph L3 ["【Layer 3】 抽象データロジック＆物理演算層 (ハードウェア非依存)"]
        L3_Struct["struct LogData ＆ volatile 共有グローバル変数群<br>(計54項目のフライトログデータ)"]
        L3_Shared["struct SharedSensorData<br>(気圧・温度・超音波・差圧スナップショット)"]
        L3_Alt["calculate_altitude() / calculate_bmp_altitude()<br>国際標準大気公式による気圧高度計算＆中央値選択"]
        L3_Spd["calculate_airspeed() / correct_airspeed()<br>ベルヌーイ公式による真対気速度計算＆二次補正"]
        L3_TO["is_takeoff()<br>TSD20 LiDAR 高度移動平均による離陸確定判定"]
    end

    subgraph L2 ["【Layer 2】 中間ラッパー＆タスク・マルチコア同期制御層"]
        L2_C0["loop() / core0_timer_callback()<br>Core 0 100Hz ループ制御"]
        L2_UpSens["update_air_bmp() / AS5600() / SDP() / gps()<br>自基板センサー読取・同期統括"]

        L2_Sync["multicore_fifo_push / pop_blocking()<br>コア間更新シグナル受け渡し"]

        L2_C1["loop1() / core1_timer_callback()<br>Core 1 100Hz ループ制御"]
        L2_RxUART["receiveUnderLog() / FslgLog() / IcsAngle()<br>他基板からの受信パース・生存判定"]
        L2_Cmd["handleEspSignal()<br>Xiao からのシリアルコマンド検知・転送"]
        L2_Tx["transmitLog() / transmitLog_for_fslg()<br>54項目 4分割 CSV フォーマット化"]
    end

    subgraph L1 ["【Layer 1】 ハードウェアドライバ＆ペリフェラルアクセス層"]
        L1_Sens["read_bmp() / read_AS5600() / read_SDP() / read_gps()<br>I2C・UART 経由の生データ取得"]
        L1_UART_Rx["TORICA_UART::readUART() / listenUART() / TORICA_ICS::read_Angle()<br>UART フレーム受信・文字列バッファ解析"]
        L1_Init["initUART() / initUART_DMA() / transmitHeader()<br>通信ペリフェラル初期化＆CSVヘッダー一括送信"]
        L1_WDT["watchdog_update()<br>ハードウェア WDT カウンタクリア"]
    end

    subgraph L0 ["【Layer 0】 物理ハードウェア＆RP2040 OS/カーネルプリミティブ"]
        L0_HW["RP2040 デュアルコア CPU (Core 0 / Core 1)<br>& 物理ピン (Wire:SDA20/SCL21, Wire1:SDA26/SCL27, UART1/2, SerialPIO)"]
        L0_Sync_HW["ハードウェア Mutex (sensor_mutex)<br>& multicore_fifo レジスタ & 100Hz ハードウェアタイマー"]
        L0_DMA["RP2040 DMA コントローラ (DREQ_UART1_TX)<br>& Watchdog ハードウェア (2000ms)"]
    end

    L2_C0 -->|"タイマー駆動"| L2_UpSens
    L2_UpSens -->|"ドライバ呼び出し"| L1_Sens
    L1_Sens --- L0_HW
    L2_UpSens -->|"ロック・コピー"| L3_Shared
    L2_UpSens -->|"シグナルプッシュ"| L2_Sync
    L2_Sync --- L0_Sync_HW
    L2_C0 -->|"core1_alive 確認で WDT リフレッシュ"| L1_WDT
    L1_WDT --- L0_DMA

    L2_C1 -->|"タイマー駆動"| L2_RxUART
    L2_C1 -->|"コマンド検知"| L2_Cmd
    L2_RxUART & L2_Cmd -->|"ドライバ呼び出し"| L1_UART_Rx
    L1_UART_Rx --- L0_HW
    L2_RxUART -->|"受信データ反映"| L3_Struct

    L2_C1 -->|"FIFOチェック・ポップ"| L2_Sync
    L2_C1 -->|"共有データから安全コピー"| L3_Shared
    L2_C1 -->|"計算指示"| L3_Alt & L3_Spd & L3_TO
    L3_Alt & L3_Spd & L3_TO -->|"計算結果・フラグ代入"| L3_Struct

    L2_C1 -->|"送信生成"| L2_Tx
    L2_Tx -->|"グローバル参照"| L3_Struct
    L2_Tx -->|"DMA非同期 / 同期転送指示"| L0_DMA & L0_HW
```
