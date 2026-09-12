#pragma once
#include "Arduino.h"
#include "SPI.h"
// สตับของไลบรารีทัช XPT2046 สำหรับตรวจคอมไพล์บน PC
class TS_Point {
public:
  int16_t x = 0, y = 0, z = 0;
};
class XPT2046_Touchscreen {
public:
  XPT2046_Touchscreen(uint8_t cs, uint8_t irq = 255) { (void)cs; (void)irq; }
  bool begin() { return true; }
  bool begin(SPIClass &sp) { (void)sp; return true; }
  void setRotation(uint8_t) {}
  bool touched() { return g_touched; }
  TS_Point getPoint() { return g_point; }
  static bool g_touched;
  static TS_Point g_point;
};
