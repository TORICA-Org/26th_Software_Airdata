# 26th_Airdata
メイン兼エアデータセンサー部（今後、エアデータと言います）のリポジトリです。

### 詳説　～26thエアデータプログラムのすべて～
Under construction


### ピン割り当てと動作確認
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
    - [x] GPS
    - [x] スピーカー
    - [x] LED
  - Xiao
    - [ ] SD
  - マイコン間通信
    - [ ] Pico-Xiao(Air)
    - [ ] Pico-Xiao(Under)

## メモ（2025/11/04更新）
- フローチャートを完成させる．基本構造は25代のプログラムを踏襲．
- 変数名の宣言は別ファイル（parameters.h）へ．プログラム冒頭で長々と宣言しない．extern使う→Githubの"26th_Software_Sample"参照
- 27代に振る予定の仕事
  - ~~GPSの受信頻度を変更．10Hzへ．~~　→GPS表面実装へ．標準で10Hz出るので，いったんは基板完成＆受信ソフトウェア作成に注力．
  - テレメトリ周り．
  - SDカードの使い方覚えてもらう＆BMのSD動作確認
  - ~~空いた時間に測距センサーの使い方動作確認~~　→完了
- センサの初期化，読み取りをライブラリ化する．基本構造は，
  - センサの初期化コード　（名前はTORICA_センサ名_init）
  - 値の読み取りコード　（名前はTORICA_センサ名_get値の名前）
    - AS5600→関数化するメリットあまりなさそう．一旦そのまま使おうかな
  - ライブラリは部品単体の状態でも動くようなものを目指す．本番用プログラムのみで動くとかはだめ．基本機能を数行の記述だけで使えるような使い勝手の良いものを目指す．
    
- 各種数値計算コードの関数化
  - これから書く
  - センサのライブラリ同様に，なるべくそのライブラリ単体で動くようなものを目指す．ただ大量のグローバル変数を使うやつは仕方がないかな．．．


- 変数名は図1の命名規則に従う．センサー類の場合，
  
  data_場所_センサー名_データ種類_単位
  
  例：data_air_dps_pressure_hPa
- delay()を使わない．delayを使うとその間処理がストップする．
- BMP390はアドレス指定必須
- 操縦桿のニュートラル調整用基板はメインの基板とは別に，またそこに動作確認用LEDを仕込む．動作確認用LEDはシリアル接続＆チェーン接続が可能なもので．
- （先の話）Xiaoのwebサーバー，アクセス制御とかの工夫しないとマイコンの処理重くなる説．テレメトリはプラホ上のみで見れたらよい
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
