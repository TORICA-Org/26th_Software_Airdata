# 26th_Airdata
メイン兼エアデータセンサー部（今後、エアデータと言います）のリポジトリです。

### 詳説　～26thエアデータプログラムのすべて～
Under construction


### ピン割り当てと動作確認（ブレッドボード）
- Pico
  - i2c0 : GP20,21 SDP810, AS5600
  - i2c1 : GP26,27 AS5600, BNO055, BMP390
  - GPS用UART: GP2(RX), GP3(TX)
  - Pico-Xiao間UART: GP16,17
  - Pico-Under間UART: GP4,5
  - スピーカー：GP15
  - LED: GP8～13
- Xiao ESP32
  - Pico-Xiao間：GP26,27
  - SD用SPI:そのまま

- 動作確認
  - Pico
    - [x] SDP810
    - [x] AS5600(i2c0)
    - [x] AS5600(i2c1)
    - [x] BNO055
    - [x] BMP390
    - [ ] GPS
    - [x] スピーカー
    - [x] LED
  - Xiao
    - [ ] SD
  - マイコン間通信
    - [x] Pico-Xiao(Air)
    - [ ] Pico-Xiao(Under)

### 試作基板のピン割り当てと動作確認
- Bico
- i2c0
  - [ ] SDP31
  - [ ] AS5600
- i2c1
  - [ ] AS5600(i2c1)
  - [ ] BNO055
  - [ ] BMP390
- UART
  - [ ] GPS RX=GP2,TX=GP3
  - [x] Xiao ESP32S3 (Bico:RX=16, TX=17, ESP:RX=44,TX=43)
  - [ ] Xiao RP2040 (Under) RX=, TX=
- [ ] speaker GP15
- LED
  - [ ] GP8
  - [ ] GP9
  - [ ] GP10
  - [ ] GP22
  - [ ] GP23
  - [ ] GP24
- [ ] SD GP11,12,13,14
- [ ] ICS RX=1


## 既知のバグ・修正すべきところ
- I2Cの接続が切れた場合，再接続させるような仕組みがない（？）現状は一度接続が切れると，再度データ取得ができていないっぽい
  - これに関連するのか，ある時急に測定周期が遅くなる．
- 時間がある程度経つと，動作が重くなる
- 

## メモ（2025/11/04更新）
- フローチャートを完成させる．基本構造は25代のプログラムを踏襲．
- ~~変数名の宣言は別ファイル（parameters.h）へ．プログラム冒頭で長々と宣言しない．extern使う→Githubの"26th_Software_Sample"参照~~　→一旦TORICA_common_includesに変数と関数をまとめた．
- 
- 27代に振る予定の仕事
  - ~~GPSの受信頻度を変更．10Hzへ．~~　→GPS表面実装へ．標準で10Hz出るので，いったんは基板完成＆受信ソフトウェア作成に注力．
  - SDカードの使い方覚えてもらう＆BMのSD動作確認
  - ~~空いた時間に測距センサーの使い方動作確認~~　→完了

- センサの初期化，読み取りをライブラリ化する．基本構造は，
  - センサの初期化コード　（名前はTORICA_センサ名_init）
  - 値の読み取りコード　（名前はTORICA_センサ名_get値の名前）
    - ~~AS5600→関数化するメリットあまりなさそう．一旦そのまま使おうかな~~
  - ライブラリは部品単体の状態でも動くようなものを目指す．本番用プログラムのみで動くとかはだめ．基本機能を数行の記述だけで使えるような使い勝手の良いものを目指す．
    
- 各種数値計算コードの関数化
  - ~~これから書く~~　とりあえず完了
  - センサのライブラリ同様に，なるべくそのライブラリ単体で動くようなものを目指す．

- 変数名は図1の命名規則に従う．センサー類の場合，
  
  data_場所_センサー名_データ種類_単位
  
  例：data_air_dps_pressure_hPa


追加でつけるもの
- 操縦桿のニュートラル調整用基板はメインの基板とは別に，またそこに動作確認用LEDを仕込む．動作確認用LEDはシリアル接続＆チェーン接続が可能なもので．

- 25,24,23代のサイト
  
  https://geode-kicker-e37.notion.site/ffff665803ce8126aa80ebe58f07f743
  
  https://telling-march-c0b.notion.site/2024-feda8acda17d414db22287306e8bd860
  
  https://771-8bit.com/blog/birdman-glider-avionics/#%E3%83%86%E3%82%B9%E3%83%88


  
<p align="center">
  <img src="https://github.com/user-attachments/assets/bee30e42-dd18-4d1b-a319-07bed593d882" width="50%">
</p>
<p align="center">
  図1　変数の命名規則と宣言場所
</p>
<br>
