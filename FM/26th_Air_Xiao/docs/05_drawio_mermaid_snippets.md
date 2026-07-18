# draw.io 用 Mermaid インポートガイド＆貼り付けコード集

このドキュメントは、作成した図解を **draw.io（app.diagrams.net）** へそのまま貼り付けてノードやエッジを自由に編集できるようにするためのガイドと、draw.io のインポート機能に最適化した純粋な Mermaid コード集です。

---

## 1. draw.io への Mermaid インポート手順

1. **draw.io (diagrams.net)** の編集画面を開きます。
2. 上部メニューの **「配置 (Arrange)」** ＞ **「挿入 (Insert)」** ＞ **「高度な設定 (Advanced)」** ＞ **「Mermaid...」** を選択します。
   （または、画面左上の **「＋（プラス）」** アイコンをクリック ＞ **「高度な設定 (Advanced)」** ＞ **「Mermaid...」** ）
3. 表示されたテキストボックスに、下の各項目 **【コードをコピー】** 内の Mermaid コードを貼り付けます（コードブロック内の文字のみ）。
4. **「挿入 (Insert)」** ボタンをクリックすると、図形オブジェクト（ノードと矢印のまとまり）としてキャンバス上に自動配置されます！
5. 配置後は、draw.io の機能で各ノードの色を変えたり、矢印の形を調整したり、テキストを自由に再編集できます。

---

## 2. draw.io 貼り付け用 Mermaid コード集

### ① システム全体の概要（全体アーキテクチャ）
*【対応箇所】: `docs/README.md`*

```mermaid
flowchart TD
    subgraph External ["外部デバイス・基板"]
        Bico["Bico基板 (Main Controller)<br>・気圧, GPS, 差圧, AoA/AoS<br>・胴体桁 IMU/BMP<br>・Under 超音波/TSD20"]
        SD_Card["microSDカード<br>(SPI接続 D0, D8, D9, D10)"]
        Web_Client["地上モニタリング端末<br>(PC/スマホ - Wi-Fi AP 'SerialWeb')"]
        Sensors["電流・電圧センサ回路<br>(LT6106電流計 / 分圧抵抗 D1, D2)"]
    end

    subgraph Core1 ["Core 1 : 高速通信・ストレージ専用 (100Hz / 10ms周期)"]
        Task_Core1["Core1_Task()"]
        UART_Listen["processCore1_ListenUART()<br>UART受信 (460,800 bps)"]
        SD_Write["processCore1_WriteSD()<br>SDバッファ追記＆フラッシュ"]
        Web_Cmd["SerialWeb_detectRESET()<br>Webコマンド受信＆Bico転送"]
    end

    subgraph Queues ["FreeRTOS キュー (Core間通信)"]
        Queue_SD["sdQueue<br> (最大5パケットバッファ)"]
        Queue_UART["uartQueue<br> (1秒毎にCore0へサンプリング転送)"]
    end

    subgraph Core0 ["Core 0 : システム解析・Web配信・センサ監視 (10Hz / 100ms周期)"]
        Task_Core0["Core0_Task()"]
        Parse_Web["processCore0_ParseAndWeb()<br>文字列パース＆変数展開"]
        Send_Web["sendSerialWeb()<br>10ステップ時分割Web配信"]
        Check_WiFi["checkAndRecoverWiFiAP()<br>Wi-Fi切断監視・自己修復"]
        Power_Check["read_voltage_V() / read_current_mA()<br>ADC電圧・電流計測"]
    end

    subgraph Globals ["共有メモリ領域 (parameters.cpp / .h)"]
        Global_Vars["54項目のエアデータ・フライトログ<br>・takeoff, time_ms<br>・GPS, BMP390, SDP31<br>・BNO055, LSM6DSV16X<br>・URM, TSD20 など"]
    end

    Bico -->|Serial1 TX 460800bps| UART_Listen
    UART_Listen -->|キュー送信| Queue_SD
    UART_Listen -->|4Hzサンプリングキュー送信| Queue_UART
    Queue_SD --> SD_Write
    SD_Write -->|SPI高速書き込み| SD_Card
    Web_Cmd -->|Serial1 TX コマンド転送| Bico

    Queue_UART --> Parse_Web
    Parse_Web -->|データ変換・格納| Global_Vars
    Global_Vars -->|データ参照| Send_Web
    Power_Check -->|計測結果参照| Send_Web
    Send_Web -->|Wi-Fiパケット送信| Web_Client
    Web_Client -->|コマンド送信 RESET/TAKEOFF等| Web_Cmd
    Sensors -->|ADC入力| Power_Check
```

---

### ② ファイル間の依存関係（インクルード結合図）
*【対応箇所】: `docs/01_file_relationships.md`*

```mermaid
flowchart TD
    subgraph Headers ["共通ヘッダー・設定層"]
        Config_H["Air_xiao_config.h<br>(ピン定義・SSID/PWD)"]
        Params_H["parameters.h<br>(グローバル変数宣言・LogData構造体)"]
    end

    subgraph Hardware_Modules ["ハードウェア制御層"]
        Power_H["power_checker.h"]
        Power_C["power_checker.cpp"]
        SD_H["SD_Air_xiao.h"]
        SD_C["SD_Air_xiao.cpp"]
        UART_H["UARTHelper_air_xiao.h"]
        UART_C["UARTHelper_air_xiao.cpp"]
        Web_H["SerialWebHelper.h"]
        Web_C["SerialWebHelper.cpp"]
    end

    subgraph Wrapper_Layer ["タスク連携・通信ブリッジ層"]
        Wrap_H["SDandUART_wrapper.h"]
        Wrap_C["SDandUART_wrapper.cpp"]
    end

    subgraph Main_Layer ["エントリポイント"]
        Main_INO["26th_Air_Xiao.ino"]
    end

    Power_C --> Power_H
    Power_C --> Config_H

    SD_H --> Params_H
    SD_C --> SD_H
    SD_C --> Config_H
    SD_C --> Params_H

    UART_H --> Params_H
    UART_C --> UART_H
    UART_C --> Params_H
    UART_C --> Config_H

    Web_H --> Params_H
    Web_H --> Config_H
    Web_C --> Web_H
    Web_C --> Power_H
    Web_C --> SD_H

    Wrap_H --> SD_H
    Wrap_H --> UART_H
    Wrap_C --> Wrap_H
    Wrap_C --> SD_H
    Wrap_C --> UART_H
    Wrap_C --> Web_H

    Main_INO --> Params_H
    Main_INO --> Web_H
    Main_INO --> Config_H
    Main_INO --> SD_H
    Main_INO --> UART_H
    Main_INO --> Power_H
    Main_INO --> Wrap_H
```

---

### ③ システム初期化・タスク起動シーケンス図 (`setup()`)
*【対応箇所】: `docs/02_core_tasks_flowchart.md`*

```mermaid
sequenceDiagram
    autonumber
    participant Setup as setup() [ino]
    participant HW as Hardware / Pins
    participant Wrap as SDandUART_wrapper
    participant Power as PowerChecker
    participant Web as SerialWebHelper
    participant RTOS as FreeRTOS Kernel

    Setup->>HW: pinMode(LED_BUILTIN, OUTPUT) & Serial.begin(115200)
    
    Note over Setup,Wrap: Core 1 用サブシステム初期化
    Setup->>Wrap: setupSDandUART()
    activate Wrap
    Wrap->>RTOS: xQueueCreate(3, sizeof(UARTData)) -> uartQueue
    Wrap->>RTOS: xQueueCreate(5, sizeof(UARTData)) -> sdQueue
    Wrap->>Wrap: initSD() : SPI.begin() & sd.begin(SD_CS)
    Wrap->>Wrap: flashHeader() : CSVヘッダー文字列のSD書き込み
    Wrap->>Wrap: initUART() : Serial1.begin(460800, SERIAL_8E1)
    deactivate Wrap

    Note over Setup,Web: Core 0 用サブシステム初期化
    Setup->>Power: init_PowerChecker() : ADC減衰率6dB設定
    Setup->>Web: init_SerialWeb() : Wi-Fi AP起動 (SSID:"SerialWeb")

    Note over Setup,RTOS: FreeRTOSマルチスレッドタスク生成
    Setup->>RTOS: xTaskCreatePinnedToCore(Core0_Task, Core 0, Stack:12288, Priority:1)
    Setup->>RTOS: xTaskCreatePinnedToCore(Core1_Task, Core 1, Stack:8192, Priority:1)
```

---

### ④ マルチコア処理・Core0 / Core1 ループとキュー間通信
*【対応箇所】: `docs/02_core_tasks_flowchart.md`*

```mermaid
flowchart TD
    subgraph Core1_Loop ["Core 1 タスク : Core1_Task() 【100Hz / 10ms周期】"]
        C1_Start(("Core 1 ループ開始")) --> C1_UART["processCore1_ListenUART()<br>1. Bico_UART.listenUART() で行データ受信<br>2. rxData.text に格納し末尾に '\n' 補完"]
        
        C1_UART -->|全受信パケット 100Hz| Send_SD_Q["xQueueSend(sdQueue, txData)"]
        
        C1_UART --> C1_Check_Counter{"1秒カウンタ >= 25 ?<br>(4Hzサンプリング)"}
        C1_Check_Counter -->|Yes| Send_UART_Q["xQueueSend(uartQueue, txData)<br>(Core 0 へサンプリング転送)"]
        C1_Check_Counter -->|No| C1_Check_Reset
        Send_UART_Q --> C1_Check_Reset{"RESET_SIG == true ?<br>(Webからリセット要求)"}
        
        C1_Check_Reset -->|Yes| Send_Reset_SD["'\nRESET\n' を sdQueue へ送信<br>RESET_SIG = false"]
        C1_Check_Reset -->|No| C1_SD_Process
        Send_Reset_SD --> C1_SD_Process
        
        C1_SD_Process["processCore1_WriteSD()<br>1. xQueueReceive(sdQueue) で取得<br>2. writeBufToSD() でバッファ追記"]
        C1_SD_Process --> C1_Check_Flash{"フラッシュカウンタ >= 5 ?<br>(5回受信毎 = 約50ms / 20Hz)"}
        
        C1_Check_Flash -->|Yes| C1_Do_Flash["writeSD()<br>sd.flash() でSDへ物理書き出し<br>正常時: LED点灯"]
        C1_Check_Flash -->|No| C1_Web_Check
        C1_Do_Flash --> C1_Web_Check
        
        C1_Web_Check{"リセット信号カウンタ > 15 ?<br>(約150ms周期)"}
        C1_Web_Check -->|Yes| C1_Detect_Web["SerialWeb_detectRESET()<br>Webからのコマンド受信 (RESET/TAKEOFF等)"]
        C1_Web_Check -->|No| C1_End(("10ms待機へ"))
        C1_Detect_Web --> C1_End
    end

    subgraph Queues_RTOS ["FreeRTOS キュー (スレッドセーフ通信)"]
        Q_SD["sdQueue バッファ: 5"]
        Q_UART["uartQueue バッファ: 3"]
    end

    subgraph Core0_Loop ["Core 0 タスク : Core0_Task() 【10Hz / 100ms周期】"]
        C0_Start(("Core 0 ループ開始")) --> C0_Parse["processCore0_ParseAndWeb()<br>1. xQueueReceive(uartQueue, 最大100ms待機)<br>2. 文字列パースと変数展開を実行"]
        C0_Parse --> C0_SendWeb["sendSerialWeb()<br>10グループ中の1グループをWi-Fi送信<br>(電圧電流計読み取り含む)"]
        C0_SendWeb --> C0_CheckWiFi["checkAndRecoverWiFiAP()<br>Wi-Fi AP IP (0.0.0.0) 監視と自己修復"]
        C0_CheckWiFi --> C0_Debug{"デバッグカウンタ > 10 ?<br>(約1秒周期)"}
        C0_Debug -->|Yes| C0_PrintStats["デバッグ統計表示<br>(最小ヒープ・スタック残量)"]
        C0_Debug -->|No| C0_End(("100ms待機へ"))
        C0_PrintStats --> C0_End
    end

    Send_SD_Q ==> Q_SD
    Send_Reset_SD ==> Q_SD
    Q_SD ==> C1_SD_Process

    Send_UART_Q ==> Q_UART
    Q_UART ==> C0_Parse
```

---

### ⑤ データ受信・パース・展開・ログ保存フローチャート
*【対応箇所】: `docs/03_data_pipeline_flowchart.md`*

```mermaid
flowchart TD
    subgraph Bico_Side ["送信元: Bico基板 (460,800 bps / 8E1 / カンマ区切り文字列)"]
        UART_Stream["生データパケット (4回分割・合計54項目)"]
    end

    subgraph Core1_Pipeline ["Core 1 : 受信＆ストレージ保存パイプライン"]
        L_UART["TORICA_UART::listenUART()<br>(末尾 '\n' を受信完了判定とし '\0' 置換)"]
        Copy_SD["snprintf(txData.text, '...\\n', Bico_UART.buff)<br>改行を復元してバッファ格納"]
        Push_SD_Queue["xQueueSend(sdQueue, txData)<br>キューへの即座転送"]
        
        Write_SD_Func["writeBufToSD(char* buffer)<br>TORICA_SD::add_str() でバッファ追記"]
        Flash_SD_Func["writeSD()<br>TORICA_SD::flash() で物理書き出し<br>(20Hz周期 / 50ms毎)"]
    end

    subgraph Core0_Pipeline ["Core 0 : パース＆構造体展開パイプライン"]
        Pop_UART_Queue["xQueueReceive(uartQueue, rxData)<br>(1秒に1回の間隔で取得)"]
        Parse_Buf["Bico_UART.parseBuffer(rxData.text)<br>カンマ区切り文字列を float配列へ変換"]
        Check_Num{"parsed_num == BICO_DATA_NUM<br>(54個すべて揃っているか？)"}
        
        Extract_Log["extractLogData(54)<br>データの妥当性確認と展開開始"]
        Convert_Log["convertArrayToLogData(Bico_UART.UART_data)<br>float配列 [0]～[53] を struct LogData へ変換"]
        Apply_Globals["applyLogDataToGlobals(LogData& data)<br>構造体から volatile グローバル変数へ一括代入"]
    end

    subgraph Global_Storage ["グローバル変数群 (parameters.cpp)"]
        G_Vars["エアデータ・IMU・高度・GPS情報<br>(volatile float/int/bool)"]
    end

    UART_Stream --> L_UART
    L_UART --> Copy_SD
    Copy_SD --> Push_SD_Queue
    Push_SD_Queue ==> Write_SD_Func
    Write_SD_Func --> Flash_SD_Func

    Copy_SD -->|25回に1回 4Hzサンプリング| Pop_UART_Queue
    Pop_UART_Queue --> Parse_Buf
    Parse_Buf --> Check_Num
    Check_Num -->|Yes| Extract_Log
    Check_Num -->|No / パケット欠落| Drop["破棄 (パースエラー時は変数更新をスキップ)"]
    Extract_Log --> Convert_Log
    Convert_Log --> Apply_Globals
    Apply_Globals ==> G_Vars
```

---

### ⑥ 10ステップ時分割テレメトリ Wi-Fi 送信フロー (`sendSerialWeb()`)
*【対応箇所】: `docs/04_web_command_flowchart.md`*

```mermaid
flowchart TD
    Start_Step(("sendSerialWeb() 呼び出し<br>【100ms周期】")) --> Check_Step{"static int step の値判定<br>(0 〜 9)"}

    Check_Step -->|step == 0| S0["ラベル: 'time_ms' / 'takeoff'<br>送信: システム時間 ms & 離陸フラグ"]
    Check_Step -->|step == 1| S1["ラベル: 'Volt, mA' / 'SD: active, file name'<br>送信: 分圧電圧(V), 負荷電流(mA), SD稼働状態＆ファイル名"]
    Check_Step -->|step == 2| S2["ラベル: 'BNO_calib: s,g,a,m'<br>送信: BNO055 キャリブレーションステータス"]
    Check_Step -->|step == 3| S3["ラベル: 'BNO_eular: roll, pitch, yaw'<br>送信: BNO055 オイラー姿勢角"]
    Check_Step -->|step == 4| S4["ラベル: 'LSM_eular: roll, pitch, yaw'<br>送信: LSM6DSV16X 6軸オイラー姿勢角"]
    Check_Step -->|step == 5| S5["ラベル: 'bmp_temp: air, under, fslg'<br>送信: BMP390 温度 (3箇所)"]
    Check_Step -->|step == 6| S6["ラベル: 'bmp_alt: air, under, fslg'<br>送信: BMP390 気圧高度 (3箇所)"]
    Check_Step -->|step == 7| S7["ラベル: 'URM, TSD20' / 'airspd'<br>送信: 超音波・TSD20高度 & SDP31対気速度"]
    Check_Step -->|step == 8| S8["ラベル: 'AoA, AoS' / 'ICS_angle'<br>送信: 迎え角・横滑り角 & ICS操舵角"]
    Check_Step -->|step == 9| S9["ラベル: 'GPS: lat, lon' / 'GPS: satellites'<br>送信: GPS緯度・経度 & 衛星捕捉数"]

    S0 --> Inc_Step
    S1 --> Inc_Step
    S2 --> Inc_Step
    S3 --> Inc_Step
    S4 --> Inc_Step
    S5 --> Inc_Step
    S6 --> Inc_Step
    S7 --> Inc_Step
    S8 --> Inc_Step
    S9 --> Inc_Step

    Inc_Step["step++ (カウントアップ)"] --> Wrap_Step{"step >= 10 ?"}
    Wrap_Step -->|Yes| Reset_Step["step = 0 へリセット"]
    Wrap_Step -->|No| End_Send(("送信処理完了<br>(各パケット間には 20ms の vTaskDelay を挿入)"))
    Reset_Step --> End_Send
```

---

### ⑦ Webクライアントからのコマンド制御シーケンス図 (`SerialWeb_detectRESET()`)
*【対応箇所】: `docs/04_web_command_flowchart.md`*

```mermaid
sequenceDiagram
    autonumber
    participant Web as 地上モニタリング画面 (Web Client)
    participant Core1 as Core 1 : SerialWeb_detectRESET()
    participant Glob as グローバル変数
    participant UART as Serial1 (Bico基板)
    participant SD as microSDカードログ

    Web->>Core1: コマンド文字列送信 (5文字以上)
    Note over Core1: CRLF (`\r`, `\n`) を除去して大文字小文字を無視判定

    alt コマンド == "RESET"
        Core1->>Glob: RESET_SIG = true
        Core1->>Web: "=====RESET=====" (Logタブへエコーバック)
        Core1->>UART: Serial1.println("RESET") -> Bico基板をリセット
        Note over Core1,SD: processCore1_ListenUART() が RESET_SIG を検知し<br>SDカードへ "\nRESET\n" 行を書き込んだ後に RESET_SIG = false に復帰
    else コマンド == "TAKEOFF"
        Core1->>Web: "=========CHANGE TAKEOFF FLAG========="
        Core1->>UART: Serial1.println("CHG_TO") -> Bico基板に離陸判定反転を通知
        Core1->>Glob: takeoff = !takeoff (フラグ反転)
    else コマンド == "CALIB"
        Core1->>Web: "=========IMU Calibration========="
        Core1->>UART: Serial1.println("CALIB") -> Bico基板のIMUゼロ点合わせを実行
    else コマンド == "SPKON" / "SPKOFF"
        Core1->>Web: "=========SPEAKER ENABLED / DISABLED========="
        Core1->>UART: Serial1.println("SPK_EN") または ("SPK_DIS") -> スピーカーON/OFF
    end
```

---

### ⑧ 電圧・電流計測の回路構成図 (`power_checker.cpp`)
*【対応箇所】: `docs/04_web_command_flowchart.md`*

```mermaid
flowchart LR
    subgraph Volt_Circuit ["電圧測定回路 (分圧回路 : GPIO D2 / ADC 6dB)"]
        V_Bat["バッテリー電圧 (V_input)"] -->|R1 = 10kΩ| Div_Node["分圧点 V_ADC"]
        Div_Node -->|R2 = 1.8kΩ| GND1["GND"]
        Div_Node -->|"read_voltage_V()"| ADC_V["analogReadMilliVolts(D2)<br>V_input = V_ADC * (R1 + R2) / R2"]
    end

    subgraph Curr_Circuit ["電流測定回路 (LT6106 ハイサイド電流アンプ : GPIO D1 / ADC 6dB)"]
        I_Load["負荷電流 (I_LOAD)"] -->|シャント抵抗 R_SENSE = 0.1Ω| Amp["LT6106 電流検出アンプ<br>・R_IN = 100Ω<br>・R_OUT = 2kΩ"]
        Amp -->|出力電圧 V_OUT| ADC_I["analogReadMilliVolts(D1)<br>I_LOAD = V_OUT * (R_IN / (R_OUT * R_SENSE))"]
    end
```

---

### ⑨ ソフトウェアレイヤー構造・関数ヒエラルキーと処理関係
*【対応箇所】: `docs/06_layer_hierarchy_flowchart.md`*

```mermaid
flowchart TD
    subgraph L3 ["【Layer 3】 抽象データロジック＆アプリケーション層 (ハードウェア非依存)"]
        L3_Struct["struct LogData & volatile グローバル変数群<br>(54項目のエアデータ・フライトログ)"]
        L3_Parse["Bico_UART.parseBuffer()<br>CSV文字列を float配列へパース"]
        L3_Convert["convertArrayToLogData()<br>float配列を LogData構造体へ変換"]
        L3_Apply["applyLogDataToGlobals()<br>構造体から volatile 変数へ一括反映"]
        L3_Extract["extractLogData()<br>項目数チェック＆データ変換統括"]
        L3_Header["flashHeader() / addDataToSDBuf()<br>CSVヘッダー生成・フォーマット変換"]
    end

    subgraph L2 ["【Layer 2】 中間ラッパー＆タスク制御層 (マルチコア同期・キュー統括)"]
        L2_C1["Core1_Task()<br>Core 1 ループ (100Hz)"]
        L2_L_UART["processCore1_ListenUART()<br>UART受信バッファのキュー振り分け"]
        L2_W_SD["processCore1_WriteSD()<br>SDキューの取り出し＆書き込み統括"]
        L2_Cmd["SerialWeb_detectRESET()<br>Webコマンド検知＆Bico制御判定"]

        L2_C0["Core0_Task()<br>Core 0 ループ (10Hz)"]
        L2_P_Web["processCore0_ParseAndWeb()<br>UARTキュー取り出し＆パース指示"]
        L2_S_Web["sendSerialWeb()<br>10ステップ時分割テレメトリ生成"]
    end

    subgraph L1 ["【Layer 1】 ハードウェアドライバ＆I/Oアクセス層 (ペリフェラル制御)"]
        L1_UART["initUART() / Bico_UART.listenUART()<br>Serial1 (460800bps 8E1) 物理受信"]
        L1_SD["initSD() / writeBufToSD() / writeSD()<br>TORICA_SD (SPI) 物理書き出し＆LED点灯"]
        L1_Power["init_PowerChecker() / read_voltage_V() / read_current_mA()<br>ESP32 ADCミリボルト変換＆分圧・電流計算"]
        L1_WiFi["initSerialWeb() / checkAndRecoverWiFiAP()<br>SerialWeb Wi-Fi AP (SSID:SerialWeb) 制御・自動修復"]
    end

    subgraph L0 ["【Layer 0】 物理ハードウェア＆FreeRTOS OSプリミティブ"]
        L0_HW["XIAO ESP32 S3 物理ピン＆ペリフェラル<br>(UART: GPIO43/44, SPI: D0/D8/D9/D10, ADC: D1/D2, AP無線)"]
        L0_RTOS["FreeRTOS キュー＆スケジューラ<br>(uartQueue / sdQueue / vTaskDelayUntil)"]
    end

    L2_C1 --> L2_L_UART
    L2_C1 --> L2_W_SD
    L2_C1 --> L2_Cmd

    L2_L_UART -->|"UART受信呼び出し"| L1_UART
    L1_UART --- L0_HW
    L2_L_UART -->|"パケット送信"| L0_RTOS
    L0_RTOS -->|"パケット受信"| L2_W_SD
    L2_W_SD -->|"バッファ追記＆フラッシュ"| L1_SD
    L1_SD --- L0_HW
    L2_Cmd -->|"シリアルコマンド送信"| L1_UART

    L2_C0 --> L2_P_Web
    L2_C0 --> L1_WiFi
    L1_WiFi --- L0_HW

    L0_RTOS -->|"UARTキュー取得"| L2_P_Web
    L2_P_Web -->|"文字列解析指示"| L3_Parse
    L2_P_Web -->|"展開指示"| L3_Extract
    L3_Extract --> L3_Convert
    L3_Convert --> L3_Apply
    L3_Apply --> L3_Struct

    L2_C0 --> L2_S_Web
    L2_S_Web -->|"グローバル変数参照"| L3_Struct
    L2_S_Web -->|"電圧電流ADC読み取り"| L1_Power
    L1_Power --- L0_HW
    L2_S_Web -->|"Wi-Fiパケット送信"| L1_WiFi
```
