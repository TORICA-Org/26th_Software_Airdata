#pragma once
#include <Arduino.h>
#include <stdint.h>

class Teseo_LIV3FL {
public:
	explicit Teseo_LIV3FL(Stream& serialPort);
	void setBaudrate(int bps);
    void reboot(void);
    void save(void);
    void coldstart(void);

private:
	Stream& serial;
	static String calc_checksum(const char* cmd);
};