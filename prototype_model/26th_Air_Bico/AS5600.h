/*---------------------------------------------------------

このファイルの役割：AS5600の値読み取り
最終更新日：2026/01/27 17:34
更新内容：ファイル作成

---------------------------------------------------------*/

#pragma once // インクルードガード（複数回読み込まれないようにする）

#include <Arduino.h>
#include <Wire.h>
#include <AS5600.h>

bool AS5600_init(void);

void read_AS5600(void);

// float AS5600_getAoS(void);
// float AS5600_getAoA(void);