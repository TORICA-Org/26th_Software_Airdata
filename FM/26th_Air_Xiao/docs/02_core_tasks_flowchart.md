# マルチコア処理・FreeRTOSタスクフロー

本ドキュメントでは、XIAO ESP32 S3 で動作する FreeRTOS マルチスレッド構成の初期化（`setup()`）から、Core 0 および Core 1 に割り当てられた独立タスクの内部ループ構造、キューによるコア間通信の詳細を解説します。

---

## 1. システム初期化・タスク起動フロー (`setup()`)

プログラム起動時、`26th_Air_Xiao.ino` の `setup()` 関数が実行され、ハードウェアピンの初期化、ペリフェラル（SDカード・UART・ADC・Wi-Fi AP）の設定が行われた後、2つの FreeRTOS タスクが異なる CPU コアに固定（Pinned）して生成されます。

```mermaid
sequenceDiagram
    autonumber
    participant Setup as setup() [ino]
    participant HW as Hardware / Pins
    participant Wrap as SDandUART_wrapper
    participant Power as PowerChecker
    participant Web as SerialWebHelper
    participant RTOS as FreeRTOS Kernel

    Setup->>HW: pinMode(LED_BUILTIN, OUTPUT)<br>Serial.begin(115200)
    
    Note over Setup,Wrap: Core 1 用サブシステム初期化
    Setup->>Wrap: setupSDandUART()
    activate Wrap
    Wrap->>RTOS: xQueueCreate(3, sizeof(UARTData)) -> uartQueue (Core0用)
    Wrap->>RTOS: xQueueCreate(5, sizeof(UARTData)) -> sdQueue (Core1 SD用)
    Wrap->>Wrap: initSD() : SPI.begin() & sd.begin(SD_CS)
    Wrap->>Wrap: flashHeader() : CSVヘッダー文字列のSD初期書き込み
    Wrap->>Wrap: initUART() : Serial1.begin(460800, SERIAL_8E1, RX:44, TX:43)
    deactivate Wrap

    Note over Setup,Web: Core 0 用サブシステム初期化
    Setup->>Power: init_PowerChecker() : ADC減衰率6dB設定
    Setup->>Web: init_SerialWeb() : Wi-Fi AP起動 (SSID:"SerialWeb") & TxPower最大化 & 省電力OFF

    Note over Setup,RTOS: FreeRTOSマルチスレッドタスク生成
    Setup->>RTOS: xTaskCreatePinnedToCore(Core0_Task, Core 0, Stack:12288, Priority:1)
    Setup->>RTOS: xTaskCreatePinnedToCore(Core1_Task, Core 1, Stack:8192, Priority:1)
    
    Note right of Setup: loop() は空関数となり<br>1秒毎の vTaskDelay() のみ実行
```

---

## 2. Core 0 vs Core 1 の処理フローチャートとキュー間通信

XIAO ESP32 S3 は、高負荷な「SDカードへの物理書き込み・UART受信」を Core 1 で専有させ、遅延や処理時間のゆらぎが大きい「文字列の浮動小数点パース・Wi-Fi通信」を Core 0 で行うことで、データ欠損のない堅牢なフライトレコーダーを実現しています。

```mermaid
graph TD
    subgraph Core1_Loop ["Core 1 タスク : Core1_Task() <br>【実行周期: 100Hz / 10ms (vTaskDelayUntil)】"]
        C1_Start(("Core 1 ループ開始")) --> C1_UART["processCore1_ListenUART()<br>1. Bico_UART.listenUART() で行データ受信<br>2. rxData.text に格納し末尾に '\n' 補完"]
        
        C1_UART -->|全受信パケット<br>100Hz| Send_SD_Q["xQueueSend(sdQueue, txData)<br>(待機なし 0ms)"]
        
        C1_UART --> C1_Check_Counter{"1秒カウンタ >= 25 ?<br>(25パケットに1回 / 4Hz)"}
        C1_Check_Counter -->|Yes| Send_UART_Q["xQueueSend(uartQueue, txData)<br>(Core 0 へサンプリング転送)"]
        C1_Check_Counter -->|No| C1_Check_Reset
        Send_UART_Q --> C1_Check_Reset{"RESET_SIG == true ?<br>(Webからリセット要求)"}
        
        C1_Check_Reset -->|Yes| Send_Reset_SD["'\nRESET\n' を sdQueue へ送信<br>RESET_SIG = false"]
        C1_Check_Reset -->|No| C1_SD_Process
        Send_Reset_SD --> C1_SD_Process
        
        C1_SD_Process["processCore1_WriteSD()<br>1. xQueueReceive(sdQueue) でデータ取得<br>2. writeBufToSD() で内部バッファへ追記"]
        C1_SD_Process --> C1_Check_Flash{"フラッシュカウンタ >= 5 ?<br>(5回受信毎 = 約50ms / 20Hz)"}
        
        C1_Check_Flash -->|Yes| C1_Do_Flash["writeSD()<br>sd.flash() でSDへ物理書き出し<br>正常時: LED点灯"]
        C1_Check_Flash -->|No| C1_Web_Check
        C1_Do_Flash --> C1_Web_Check
        
        C1_Web_Check{"リセット信号カウンタ > 15 ?<br>(約150ms周期)"}
        C1_Web_Check -->|Yes| C1_Detect_Web["SerialWeb_detectRESET()<br>Webからのコマンド受信 (RESET, TAKEOFF等)"]
        C1_Web_Check -->|No| C1_End(("10ms待機へ"))
        C1_Detect_Web --> C1_End
    end

    subgraph Queues_RTOS ["FreeRTOS キュー (スレッドセーフ通信)"]
        Q_SD["sdQueue<br>バッファサイズ: 5"]
        Q_UART["uartQueue<br>バッファサイズ: 3"]
    end

    subgraph Core0_Loop ["Core 0 タスク : Core0_Task() <br>【実行周期: 10Hz / 100ms (vTaskDelayUntil)】"]
        C0_Start(("Core 0 ループ開始")) --> C0_Parse["processCore0_ParseAndWeb()<br>1. xQueueReceive(uartQueue, 最大100ms待機)<br>2. 文字列パースと変数展開を実行"]
        C0_Parse --> C0_SendWeb["sendSerialWeb()<br>10グループ中の1グループをWi-Fi送信<br>(電圧電流計読み取り含む)"]
        C0_SendWeb --> C0_CheckWiFi["checkAndRecoverWiFiAP()<br>Wi-Fi AP IP (0.0.0.0) 監視と自己修復"]
        C0_CheckWiFi --> C0_Debug{"デバッグカウンタ > 10 ?<br>(約1秒周期)"}
        C0_Debug -->|Yes| C0_PrintStats["デバッグ統計表示<br>(最小ヒープ・スタック残量)"]
        C0_Debug -->|No| C0_End(("100ms待機へ"))
        C0_PrintStats --> C0_End
    end

    %% キュー接続
    Send_SD_Q ===> Q_SD
    Send_Reset_SD ===> Q_SD
    Q_SD ===> C1_SD_Process

    Send_UART_Q ===> Q_UART
    Q_UART ===> C0_Parse
```

---

## 3. タスクパラメータと設計上の工夫

- **スタックサイズとヒープメモリ保護**
  - ESP32 S3 の FreeRTOS は十分なスタックサイズが要求されます。`Core0_Task`（文字列解析・Wi-Fi）には `12288 バイト`、`Core1_Task`（SPI通信・UART）には `8192 バイト` のスタックが割り振られています。
  - キュー (`uartQueue`, `sdQueue`) に入る構造体 `UARTData` は `2048 バイト` のテキストバッファ (`char text[2048]`) を持つため、キューの深さをそれぞれ `3` と `5` に抑え、SRAM の枯渇（ヒープオーバーフロー）を防いでいます。

- **`vTaskDelayUntil` による定周期保証**
  - 通常の `vTaskDelay` では関数の実行時間分だけ次の周期が後ろにズレていきますが、本コードでは `vTaskDelayUntil` を使用し、タスクの開始時刻を基準とした **正確な 10ms / 100ms 周期** で処理を回しています。
