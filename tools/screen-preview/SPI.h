#pragma once
#include "Arduino.h"
#define FSPI 0
class SPIClass { public: SPIClass(int=0){} void begin(int,int,int,int){} };
inline SPIClass SPI(0);
