# SerialWeb通信・コマンド制御・電力計測

本ドキュメントでは、XIAO ESP32 S3 が提供する Wi-Fi Access Point (`SSID: SerialWeb`) を介したテレメトリデータの時分割マルチプレクス送信 (`sendSerialWeb()`) と、Webクライアント側からの制御コマンド受信 (`SerialWeb_detectRESET()`)、および搭載されている電流・電圧センサ計 (`power_checker`) の仕組みを解説します。

---

## 1. 10ステップ時分割マルチプレクス送信フロー (`sendSerialWeb()`)

ESP32 S3 から Wi-Fi 経由で大量の文字列データを一度にバースト送信すると、ネットワーク輻輳やスタックオーバーフロー、パケットドロップが発生する危険性があります。
これを防ぐため、`SerialWebHelper.cpp` の `sendSerialWeb()` では、10Hz (100ms周期) で動作する Core 0 タスクの中で、**「1回のタスク実行につき 1グループ（ステップ 0～9）のデータだけを順番に送信する時分割方式（マルチプレクサ）」** を採用しています。全10ステップが1秒間に1サイクルのペースで一巡します。

```mermaid
graph TD
    Start_Step(("sendSerialWeb() 呼び出し<br>【100ms周期】")) --> Check_Step{"static int step の値判定<br>(0 〜 9)"}

    Check_Step -->|step == 0| S0["ラベル: 'time_ms' / 'takeoff'<br>送信: システム時間 ms & 離陸フラグ"]
    Check_Step -->|step == 1| S1["ラベル: 'Volt, mA' / 'SD: active, file name'<br>送信: 分圧電圧(V), 負荷電流(mA), SD稼働状態＆ファイル名"]
    Check_Step -->|step == 2| S2["ラベル: 'BNO_calib: s,g,a,m'<br>送信: BNO055 キャリブレーションステータス (システム,ジャイロ,加速度,地磁気)"]
    Check_Step -->|step == 3| S3["ラベル: 'BNO_eular: roll, pitch, yaw'<br>送信: BNO055 オイラー姿勢角 (スタック節約のため整数部と小数部を個別フォーマット)"]
    Check_Step -->|step == 4| S4["ラベル: 'LSM_eular: roll, pitch, yaw'<br>送信: LSM6DSV16X 6軸オイラー姿勢角"]
    Check_Step -->|step == 5| S5["ラベル: 'bmp_temp: air, under, fslg'<br>送信: BMP390 温度 (Air, Under, 胴体桁の3箇所)"]
    Check_Step -->|step == 6| S6["ラベル: 'bmp_alt: air, under, fslg'<br>送信: BMP390 気圧高度 (Air, Under, 胴体桁の3箇所)"]
    Check_Step -->|step == 7| S7["ラベル: 'URM, TSD20' / 'airspd'<br>送信: 超音波・TSD20高度 & SDP31対気速度(m/s)"]
    Check_Step -->|step == 8| S8["ラベル: 'AoA, AoS' / 'ICS_angle'<br>送信: 迎え角・横滑り角(deg) & ICS操舵角"]
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

## 2. Webクライアントからのコマンド受信＆制御フロー (`SerialWeb_detectRESET()`)

Core 1 タスクでは、15サイクル (約150ms) おきに `SerialWeb.available()` がチェックされ、地上端末（PC・スマートフォン）の Web GUI から送信された文字列コマンドをトリム＆パースして、即座にマイコン変数や Bico 基板へ反映します。

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

## 3. Wi-Fi AP 自己修復機構 (`checkAndRecoverWiFiAP()`)

フライト中の電波干渉や電圧変動により Wi-Fi Access Point がハングアップした場合に備え、Core 0 タスクは `loop()` 代わりの無限ループ内で毎回 `checkAndRecoverWiFiAP()` を呼び出しています。

1. `WiFi.softAPIP()` で自機の AP IP アドレスを確認します。
2. もし IP アドレスが `0.0.0.0` にダウンしていた場合、以下を自動実行して通信を完全復旧させます。
   - `WiFi.softAPdisconnect(true)` で Wi-Fi スタックを一度リセット
   - `WiFi.mode(WIFI_AP)` に明示設定
   - `WiFi.softAP(SSID, PASSWORD, 1, 0, 1)` で AP を再構築（SSID: `SerialWeb` / CH: 1 / 最大接続: 1）
   - `WiFi.setSleep(false)` および `esp_wifi_set_ps(WIFI_PS_NONE)` を再適用し、省電力スリープによる遅延やパケット詰まりを排除。

---

## 4. 電圧・電流計測回路と計算式 (`power_checker.cpp`)

XIAO ESP32 S3 に接続されたアナログピンを介して、機体の電源系統の監視（バッテリー電圧および消費電流の測定）を行っています。

```mermaid
graph LR
    subgraph Volt_Circuit ["電圧測定回路 (分圧回路 : GPIO D2 / ADC 6dB)"]
        V_Bat["バッテリー電圧 (V_input)"] -->|R1 = 10kΩ| Div_Node["分圧点 V_ADC"]
        Div_Node -->|R2 = 1.8kΩ| GND1["GND"]
        Div_Node -->|"read_voltage_V()"| ADC_V["analogReadMilliVolts(D2)<br>V_input = V_ADC * (R1 + R2) / R2"]
    end

    subgraph Curr_Circuit ["電流測定回路 (LT6106 ハイサイド電流アンプ : GPIO D1 / ADC 6dB)"]
        I_Load["負荷電流 (I_LOAD)"] -->|シャント抵抗<br>R_SENSE = 0.1Ω| Amp["LT6106 電流検出アンプ<br>・R_IN = 100Ω<br>・R_OUT = 2kΩ"]
        Amp -->|出力電圧 V_OUT| ADC_I["analogReadMilliVolts(D1)<br>I_LOAD = V_OUT * (R_IN / (R_OUT * R_SENSE))"]
    end
```

- **ADC減衰率設定** : `init_PowerChecker()` において `analogSetAttenuation(ADC_6db)` を設定し、ESP32のミリボルト読み取り精度を最大化しています。
- **電圧算出関数 (`read_voltage_V()`)**
  $$V_{\text{bat}} = V_{\text{ADC}} \times \frac{R_1 + R_2}{R_2} = V_{\text{ADC}} \times \frac{10\text{k}\Omega + 1.8\text{k}\Omega}{1.8\text{k}\Omega}$$
- **電流算出関数 (`read_current_mA()`)**
  $$I_{\text{load}} (\text{A}) = V_{\text{out}} \times \frac{R_{\text{in}}}{R_{\text{out}} \times R_{\text{sense}}} = V_{\text{out}} \times \frac{100\Omega}{2000\Omega \times 0.1\Omega} = V_{\text{out}} \times 0.5$$
  これを $\times 1000$ して $\text{mA}$ 単位に変換し、リアルタイムに Web 画面へ送信しています。
