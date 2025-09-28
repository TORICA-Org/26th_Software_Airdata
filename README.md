# 26th_Airdata
メイン兼エアデータセンサー部（今後、エアデータと言います）のリポジトリです。


## メモ（2025/09/29更新）
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
  
<p align="center">
  <img src="https://github.com/user-attachments/assets/bee30e42-dd18-4d1b-a319-07bed593d882" width="50%">
</p>
<p align="center">
  図1　変数の命名規則と宣言場所
</p>
<br>
