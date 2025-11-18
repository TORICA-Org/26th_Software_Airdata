#include "TORICA_AS5600.h"
#include <math.h> 

static AS5600 *aos_dev = nullptr;
static AS5600 *aoa_dev = nullptr;
static uint8_t aos_addr = 0x36;
static uint8_t aoa_addr = 0x36;
static TwoWire *aos_wire = nullptr;
static TwoWire *aoa_wire = nullptr;

static bool i2c_ping(TwoWire &wire, uint8_t address) {
  wire.beginTransmission(address);
  uint8_t err = wire.endTransmission();
  return (err == 0);
}

bool TORICA_AS5600_AoS_init(TwoWire &wire, uint8_t address) {
  aos_addr = address;
  aos_wire = &wire;


  if (!i2c_ping(wire, address)) {
    return false;
  }

  if (aos_dev) {
    delete aos_dev;
    aos_dev = nullptr;
  }


  aos_dev = new AS5600(aos_wire);
  if (!aos_dev) {
    return false;
  }

  return true;
}

bool TORICA_AS5600_AoA_init(TwoWire &wire, uint8_t address) {
  aoa_addr = address;
  aoa_wire = &wire;

  if (!i2c_ping(wire, address)) {
    return false;
  }

  if (aoa_dev) {
    delete aoa_dev;
    aoa_dev = nullptr;
  }

  aoa_dev = new AS5600(aoa_wire);
  if (!aoa_dev) {
    return false;
  }

  return true;
}

float TORICA_AS5600_getAoS(void) {
  if (!aos_dev) return NAN;


  bool magnet_ok = false;
  float angle_deg = 0.0;

  if (aos_dev->detectMagnet()) {
    float raw = aos_dev->rawAngle();
    angle_deg = raw * AS5600_RAW_TO_DEGREES;
    magnet_ok = true;
  } else {
    magnet_ok = false;
  }

  return magnet_ok ? angle_deg : 0.0;
}

float TORICA_AS5600_getAoA(void) {
  if (!aoa_dev) return 0.0;
  bool magnet_ok = false;
  float angle_deg = 0.0;
  if (aoa_dev->detectMagnet()) {
    float raw = aoa_dev->rawAngle();
    angle_deg = raw * AS5600_RAW_TO_DEGREES;
    magnet_ok = true;
  } else {
    magnet_ok = false;
  }

  return magnet_ok ? angle_deg : 0.0;
}