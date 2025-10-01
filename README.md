# 26th_Airdata
メイン兼エアデータセンサー部（今後、エアデータと言います）のリポジトリです。

### 詳説　～26thエアデータプログラムのすべて～
Under construction


### センサ別ピン割り当てと動作確認
- Pico
  - i2c0 : GP20,21 SDP810, AS5600
  - i2c1 : GP26,27 AS5600, BNO055, BMP390
  - GPS用UART: GP2(RX), GP3(TX)
  - Pico-Xiao間UART: GP16,17
  - Pico-Under間UART: GP4,5
  - スピーカー：GP15
  - LED: GP8～13
- Xiao RP2040
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

## メモ（2025/10/01更新）
- フローチャートを完成させる
- 変数名の宣言は別ファイル（parameters.h）へ．プログラム冒頭で長々と宣言しない．extern使う
- 変数名は図1の命名規則に従う．センサー類の場合，
  data_場所_センサー名_データ種類_単位
  例：data_air_dps_pressure_hPa
- delay()を使わない．delayを使うとその間処理がストップする．
- 25,24,23代のサイト
  
  https://geode-kicker-e37.notion.site/ffff665803ce8126aa80ebe58f07f743
  
  https://telling-march-c0b.notion.site/2024-feda8acda17d414db22287306e8bd860
  
  https://771-8bit.com/blog/birdman-glider-avionics/#%E3%83%86%E3%82%B9%E3%83%88

- BMP390はアドレス指定必須
  
<p align="center">
  <img src="https://github.com/user-attachments/assets/bee30e42-dd18-4d1b-a319-07bed593d882" width="50%">
</p>
<p align="center">
  図1　変数の命名規則と宣言場所
</p>
<br>
