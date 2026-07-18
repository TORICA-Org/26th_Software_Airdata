# ソフトウェアレイヤー構造・関数ヒエラルキーと処理関係

本ドキュメントでは、`26th_Air_Xiao` プロジェクトにおける各関数が「物理デバイスやハードウェアレジスタに近い低レイヤー関数」なのか、「タスク間通信やバッファ管理を行う中間ラッパーなのか」、「データの抽象化・変換・構造体展開を担う高レイヤー抽象関数なのか」を明確に分類した階層（ヒエラルキー）図と、階層間での処理の呼び出し関係を解説します。

---

## 1. レイヤー階層（ヒエラルキー）の定義と役割

システム全体の設計を以下の4階層に分割して整理しています。

| 階層 | レイヤー名 | 役割・責務 | 該当する主なモジュール・関数 |
| :---: | :--- | :--- | :--- |
| **Layer 3** | **抽象データロジック層**<br>*(Abstracted Data Layer)* | ハードウェアを意識しない純粋なデータ変換、文字列展開、構造体マッピング、グローバル状態保持 | `convertArrayToLogData()`, `applyLogDataToGlobals()`, `extractLogData()`, `Bico_UART.parseBuffer()`, `LogData` 構造体, 54個の `volatile` 変数 |
| **Layer 2** | **中間ラッパー＆タスク制御層**<br>*(Middleware / Task Wrapper)* | FreeRTOSタスクループ、コア間のキュー受け渡し、時分割送信制御、コマンド検知 | `Core0_Task()`, `Core1_Task()`, `processCore0_ParseAndWeb()`, `processCore1_ListenUART()`, `processCore1_WriteSD()`, `sendSerialWeb()`, `SerialWeb_detectRESET()` |
| **Layer 1** | **ハードウェアドライバ＆I/O制御層**<br>*(Hardware Driver Layer)* | SPI / UART / ADC / Wi-Fi ペリフェラル通信、ハードウェアレジスタの直接操作 | `initSD()`, `writeBufToSD()`, `writeSD()`, `initUART()`, `Bico_UART.listenUART()`, `read_voltage_V()`, `read_current_mA()`, `checkAndRecoverWiFiAP()`, `initSerialWeb()` |
| **Layer 0** | **物理ハードウェア＆RTOSカーネル**<br>*(Physical Hardware / Kernel)* | ESP32 S3 デュアルコア CPU、ペリフェラルピン、FreeRTOS OSプリミティブ | GPIOピン (`D0`〜`D10`, `43`/`44`, `D1`/`D2`), `Serial1`, SPIバス, ADC減衰器, `xQueueSend/Receive`, `vTaskDelayUntil` |

---

## 2. ソフトウェアレイヤーヒエラルキー＆処理関係の統合フローチャート

以下の図は、上段から下段へと向かう**レイヤー抽象度の高さ（上：高レイヤー・抽象ロジック ⇔ 下：低レイヤー・物理I/O）** と、その中での関数呼び出し・データフローを示しています。

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

    %% --- 処理と依存関係の流れ ---

    %% Core1の制御フロー
    L2_C1 --> L2_L_UART
    L2_C1 --> L2_W_SD
    L2_C1 --> L2_Cmd

    %% Core1からLayer1ドライバへのアクセス
    L2_L_UART -->|"UART受信呼び出し"| L1_UART
    L1_UART --- L0_HW
    L2_L_UART -->|"パケット送信"| L0_RTOS
    L0_RTOS -->|"パケット受信"| L2_W_SD
    L2_W_SD -->|"バッファ追記＆フラッシュ"| L1_SD
    L1_SD --- L0_HW
    L2_Cmd -->|"シリアルコマンド送信"| L1_UART

    %% Core0の制御フロー
    L2_C0 --> L2_P_Web
    L2_C0 --> L1_WiFi
    L1_WiFi --- L0_HW

    %% Core0からLayer3抽象ロジックへのデータ展開
    L0_RTOS -->|"UARTキュー取得"| L2_P_Web
    L2_P_Web -->|"文字列解析指示"| L3_Parse
    L2_P_Web -->|"展開指示"| L3_Extract
    L3_Extract --> L3_Convert
    L3_Convert --> L3_Apply
    L3_Apply --> L3_Struct

    %% Web送信での階層連携
    L2_C0 --> L2_S_Web
    L2_S_Web -->|"グローバル変数参照"| L3_Struct
    L2_S_Web -->|"電圧電流ADC読み取り"| L1_Power
    L1_Power --- L0_HW
    L2_S_Web -->|"Wi-Fiパケット送信"| L1_WiFi
```

---

## 3. レイヤー間の分離メリットと設計上の特長

1. **Layer 3（抽象データロジック）の独立性**
   - `LogData` 構造体や `convertArrayToLogData()` などは、通信速度やハードウェアピン、SPI の物理的な制約を一切意識せず、「float型の配列から特定の変数へ代入する」という純粋なビジネスロジックに集中しています。これにより、将来センサが増減した際も Layer 3 の変数変更のみで対応可能です。

2. **Layer 2（タスク・キューラッパー）によるコア間の調停**
   - Layer 1（ハードウェアドライバ）は本来単一のコアからアクセスされることを前提としますが、Layer 2 が `uartQueue` や `sdQueue` という Layer 0（RTOSプリミティブ）を介することで、Core 1 の高速な UART/SD 通信と Core 0 の遅いパース・Web 処理を完全に分離し、競合（データレース）を回避しています。

3. **Layer 1（ハードウェアドライバ）のカプセル化**
   - 分圧抵抗計算や電流計 LT6106 のシャント抵抗計算 (`power_checker.cpp`)、SPI CSピン選択 (`SD_Air_xiao.cpp`)、UART ボーレート設定 (`UARTHelper_air_xiao.cpp`) などの物理回路に依存するパラメータを Layer 1 に閉じ込めることで、上位レイヤーからは `read_voltage_V()` や `writeSD()` というシンプルなインターフェースで安全に呼び出せるようになっています。
