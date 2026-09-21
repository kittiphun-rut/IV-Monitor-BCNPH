#pragma once
#include "Adafruit_GFX.h"
#include "SPI.h"
class Adafruit_ST7789 : public Adafruit_GFX {
public:
  Adafruit_ST7789(SPIClass*,int,int,int){}
  void init(int,int){}
  void setSPISpeed(unsigned long){}
};
