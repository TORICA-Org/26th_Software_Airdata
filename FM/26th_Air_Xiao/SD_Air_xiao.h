/*-----------------------

このファイルの役割：XiaoでのSD用関数

------------------------*/

#pragma once

#include <SD.h>
#include <TORICA_SD.h>

#include "parameters.h"

extern bool SD_is_active;

bool initSD();

void flashHeader();

void flashSD(int flash_mode);

void addDataToSDBuf(const LogData& data, int flash_mode);

void writeSD();

void writeBufToSD(char* buffer);

bool check_SDisActive();

const char* get_SDfileName();