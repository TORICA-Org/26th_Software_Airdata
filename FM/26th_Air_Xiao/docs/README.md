# XIAO ESP32 S3 ソフトウェア仕様書（エアデータ・ロガー・Wi-Fiテレメトリ通信部）

この `docs` ディレクトリには、`26th_Air_Xiao.ino` を中核とする XIAO ESP32 S3 用プログラムの全容・ファイル間関係・関数仕様・マルチコアタスク構成を解説するドキュメントと、Mermaid形式のフローチャートを収録しています。

---

## 1. ドキュメント構成（目次）

本ドキュメントは以下の6部構成（＋draw.io貼り付け用コード集の全7ファイル）となっています。各Markdownファイル内で詳細なMermaidチャートと解説を記述しています。

| ファイル名 | タイトル | 概要 |
| :--- | :--- | :--- |
| **[`01_file_relationships.md`](01_file_relationships.md)** | **ファイル間関係とモジュール構成** | 各 `.ino`, `.h`, `.cpp` ファイルの責務、依存関係（インクルードグラフ）、データ保持・共有の仕組み |
| **[`02_core_tasks_flowchart.md`](02_core_tasks_flowchart.md)** | **マルチコア処理・FreeRTOSタスクフロー** | Core 0（解析・Web通信・電力計測）および Core 1（高速UART受信・SDログ保存）のタスク構成とキュー通信 |
| **[`03_data_pipeline_flowchart.md`](03_data_pipeline_flowchart.md)** | **データ受信・パース・展開・ログ保存フロー** | Bico基板（Main）からの54項目データ受信から構造体変換、グローバル変数への反映、SD書き込み・Web送信までの関数連携 |
| **[`04_web_command_flowchart.md`](04_web_command_flowchart.md)** | **SerialWeb通信・コマンド制御・電力計測** | Wi-Fi Access Point 経由の分割テレメトリ送信（10ステップマルチプレクス）、Webコマンド（RESET, TAKEOFF等）受信、LT6106電流・分圧電圧計測 |
| **[`06_layer_hierarchy_flowchart.md`](06_layer_hierarchy_flowchart.md)** | **レイヤー構造・関数ヒエラルキーと処理関係** | 各関数を物理I/O・ハードウェアドライバ・中間ラッパー・抽象データロジックの4階層に分類し、処理やデータアクセスの呼び出し関係を明示した階層図 |
| **[`05_drawio_mermaid_snippets.md`](05_drawio_mermaid_snippets.md)** | **draw.io用貼り付けコード＆インポート手順** | draw.io (app.diagrams.net) の Mermaid インポート機能に最適化した、そのまま1クリックで貼り付け可能な全図解コード集とインポート手順 |

---

## 2. システム全体の概要（全体アーキテクチャ）

XIAO ESP32 S3 は、エアデータ計測システムの「データ収集ハブ兼フライトデータレコーダー・地上モニタリング親機」として機能します。
FreeRTOS のマルチコア機能（Core 0 と Core 1）を最大限に活用し、**「100Hzの高速SDカード記録・UART通信」** と **「Wi-Fiテレメトリ配信・文字列パース・環境計測」** を完全にコア分離して並列実行しています。

```mermaid
graph TD
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

    %% 外部とCore1の接続
    Bico -->|Serial1 TX<br>460800bps| UART_Listen
    UART_Listen -->|キュー送信| Queue_SD
    UART_Listen -->|4Hzサンプリング<br>キュー送信| Queue_UART
    Queue_SD --> SD_Write
    SD_Write -->|SPI高速書き込み| SD_Card
    Web_Cmd -->|Serial1 TX<br>コマンド転送| Bico

    %% Core1とCore0の接続
    Queue_UART --> Parse_Web
    Parse_Web -->|データ変換・格納| Global_Vars
    Global_Vars -->|データ参照| Send_Web
    Power_Check -->|計測結果参照| Send_Web
    Send_Web -->|Wi-Fiパケット送信| Web_Client
    Web_Client -->|コマンド送信<br>RESET/TAKEOFF等| Web_Cmd
    Sensors -->|ADC入力| Power_Check
```

---

## 3. コア別タスクの責務まとめ

| コア | タスク関数 | 動作周期 | 実行順序・概要 |
| :---: | :--- | :---: | :--- |
| **Core 0** | `Core0_Task()` | **100ms (10Hz)** | 1. `processCore0_ParseAndWeb()` : キューからUART文字列を受信・パース・構造体展開しグローバル変数へ反映<br>2. `sendSerialWeb()` : 変数および `read_voltage_V()`, `read_current_mA()` を取得し、10分割のステップ順でWi-Fi送信<br>3. `checkAndRecoverWiFiAP()` : Wi-Fi AP状態が落ちていないかチェック・再起動 |
| **Core 1** | `Core1_Task()` | **10ms (100Hz)** | 1. `processCore1_ListenUART()` : Bicoからのデータ行(`\n`区切り)を受信し `sdQueue` へ転送＆周期的に `uartQueue` へコピー<br>2. `processCore1_WriteSD()` : `sdQueue` からデータを取り出してバッファへ書き込み、5回に1回(`writeSD()`)物理フラッシュ<br>3. `SerialWeb_detectRESET()` : 15サイクル毎にWebからのコマンドを確認してBicoへ転送 |

---
各機能のさらなる詳細やフローチャートについては、`docs/` 内の各Markdownファイルをご確認ください。
