#pragma once

#include <Arduino.h>

void initUART();
void receiveLog();
void extractLogData(int readnum);