# 時分割パケット配信・コマンド制御・物理計算仕様

`26th_Air_Bico` は、地上モニタリング親機（Xiao ESP32 S3）との間で高速通信を行うとともに、Xiao 経由で受信した Web クライアントの制御コマンド（リセット、離陸判定切替、スピーカーON/OFF等）を解析し、各サブシステムへと分配する制御ハブ機能を持っています。また、飛行性能を左右する高度・対気速度の物理計算をリアルタイムに実行しています。

---

## 1. 4段階マルチプレクス時分割テレメトリ送信 (`transmitLog()`)

Core 1 は 100Hz (10ms 周期) のループ内で毎回 `transmitLog(transmit_count)` を呼び出し、静的変数 `transmit_count` をインクリメントしながら 4 分割されたデータグループ (`case 0` 〜 `case 3`) を順次 DMA 送信します。4 回でちょうど 1 サイクルの全 54 項目が送信完了します（全パケット送信周期 = 40ms / 25Hz）。

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

---

## 2. Xiao / Web クライアントからのコマンド制御シーケンス (`handleEspSignal()`)

Xiao ESP32 S3 から `Serial_ESP` (`Serial2` / `ESP_UART`) にシリアル文字が届くと、Core 1 の `handleEspSignal()` が内容をパースし、対応する動作や各サブシステムへのコマンド転送を実行します。

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

---

## 3. エアデータ物理計算式と回路・処理プロセス図

`26th_Air_Bico` の中核である「気圧高度」「対気速度」「離陸判定」は、複数のセンサーを組み合わせてフィルタリングを行うことでノイズを排除しています。

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

### 気圧高度計算公式 (`calculate_bmp_altitude`)
国際標準大気 (ISA) モデルの公式に基づき、各センサーの気圧 $P$ と温度 $T$ から標高 $h$ を算出します。
$$h = \frac{\left( \left( \frac{1013.25}{P} \right)^{\frac{1}{5.257}} - 1 \right) \times (T + 273.15)}{0.0065}$$
さらに、離陸前のプラットフォーム上で測定した 50 タップ平均との差を取り、プラットフォーム固有高度 $10.6\text{m}$ (`const_platform_altitude_m`) を加算。3 系統（Air, Under, Fslg）の中で中央値を取ることで、単一センサーの突発的なスパイクノイズを完全に遮断しています。

### 対気速度計算公式 (`calculate_airspeed`)
ベルヌーイの定理と気体の状態方程式に基づき、差圧 $\Delta P$、気圧 $P$、気温 $T$、気体定数 $R/M = 287.026 \text{ J/(kg} \cdot \text{K)}$ から速度 $v$ を導出します。
$$v = \sqrt{ \left| 2 \Delta P \times \frac{T + 273.15}{P \times 100} \times 287.026 \right| }$$
これにより、高高度で気温や気圧が変化しても正確な真対気速度を獲得できます。
