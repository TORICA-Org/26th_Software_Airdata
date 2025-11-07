#include "TORICA_TSD20.h"

// Checksumの計算（ビット反転）
static uint8_t calcChecksum(const uint8_t *data, uint8_t len) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < len; ++i) sum += data[i];
  return ~sum;
}

// [0] Header 0x5C, [1] Dist_L, [2] Dist_H, [3] Checksum
// available() >= 4 のときのみ処理し、ヘッダが見つかるまでバッファを消費して同期をとる
// エラー（不足・チェックサム不一致・5000mm=out of range）は 0.0 を返す
float TORICA_TSD20_getDistance_m(Stream &serial) {
  if (serial.available() < 4) return 0.0f;
  while (serial.available() >= 4) {
    int hdr = serial.read();
    if (hdr < 0) {
      return 0.0f;
    }
    if (hdr != 0x5C) {
      continue;
    }

    int r1 = serial.read();
    int r2 = serial.read();
    int r3 = serial.read();
    if (r1 < 0 || r2 < 0 || r3 < 0) {
      return 0.0f;
    }

    uint8_t dist_L = (uint8_t)r1;
    uint8_t dist_H = (uint8_t)r2;
    uint8_t recv_chk = (uint8_t)r3;

    uint8_t data_for_chk[2] = { dist_L, dist_H };
    uint8_t calc_chk = calcChecksum(data_for_chk, 2);

    if (recv_chk != calc_chk) {
      // チェックサムエラー
      return 0.0f;
    }

    uint16_t distance_mm = (uint16_t)((dist_H << 8) | dist_L);
    if (distance_mm == 5000) {
      return 0.0f;
    }

    // mmからmに変換
    return (float)distance_mm / 1000.0f;
  }

  // ヘッダが見つからなかった、またはデータ不足
  return 0.0f;
}