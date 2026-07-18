# RP2040 デュアルコア処理・タイマー割り込み＆タスクフロー

`26th_Air_Bico` プログラムでは、RP2040 の 2 つの CPU コア（Core 0 と Core 1）にタスクを完全に分離し、それぞれに対して独立した 100Hz（周期 10ms）のハードウェアタイマー割り込みを設定して定周期処理を行っています。

---

## 1. 初期化およびマルチコア起動シーケンス (`setup()` / `setup1()`)

システム起動時、Core 0 が最初に `setup()` を実行し、全ペリフェラルの構成、センサーの初期化、CSV ヘッダーの送信を行った後、ハードウェアタイマーおよび Watchdog を有効化します。 Core 1 は RP2040 の `pico/multicore` 機構によって自動起動され、並列で `setup1()` を実行します。

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

---

## 2. Core 0 vs Core 1 のタスクループフローチャートとコア間通信

タイマーコールバック関数 (`core0_timer_callback` / `core1_timer_callback`) が 10ms 毎にフラグ (`core0_timer_triggered` / `core1_timer_triggered`) を `true` にすることで、メインループ (`loop()` / `loop1()`) 内の定周期処理が起動します。

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

    %% --- 接続関係 ---
    C0_Sync ===> FIFO_Box
    C0_Sync -.-> Mutex_Box
    FIFO_Box ===> C1_CheckFIFO
    Mutex_Box -.-> C1_PopCopy
    C1_Alive -.-> C0_CheckWDT
```

---

## 3. 設計上の特長と Watchdog による安全防護

### `add_repeating_timer_ms(-10, ...)` の「負の周期パラメータ」
- RP2040 の `pico/stdlib.h` タイマーにおいて、周期に負の数（例：`-10ms`）を指定すると、**「コールバックの処理完了にかかる時間に関わらず、前回の起動時刻を基準として正確に 10ms (100Hz) 間隔でタイマーを発火させる」** という定周期保証モードになります。これにより、計算や受信処理のゆらぎに影響されずにジッターのないサンプリングが可能となっています。

### デュアルコアクロス Watchdog 監視機構
- ESP32 などの一般的なマイコンではコア個別に WDT を持たせることが多いですが、本システムでは **「Core 0 と Core 1 がお互いを監視して初めて WDT がリフレッシュされる」** という高度な相互監視を行っています。
- Core 1 が 1 ループ実行するたびに `core1_alive = true` をセットします。
- Core 0 は自ループの最後で `core1_alive == true` を確認できた場合のみ `watchdog_update()` を実行して WDT カウンタを巻き戻し、`core1_alive = false` にリセットします。
- **効果**: もし Core 1 の UART 受信がハングアップしたり物理計算で無限ループに陥ったりした場合、Core 0 が正常に動いていても `watchdog_update()` が実行されなくなり、**2 秒経過すると自動的にシステム全体をリセット・再起動** させます。
