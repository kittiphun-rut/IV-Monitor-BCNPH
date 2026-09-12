// สตับ U8g2lib.h สำหรับตรวจไวยากรณ์เฟิร์มแวร์ Host รุ่นจอ OLED บนเครื่อง PC
// บันทึกคำสั่งวาดในรูปแบบเดียวกับสตับ Adafruit_GFX จึงนำไปเรนเดอร์เป็นภาพต่อได้
#pragma once
#include <stdint.h>
#include <cstdio>
#include "Arduino.h"

#define U8G2_R0 0
#define U8X8_PIN_NONE 255

typedef const uint8_t* u8g2_font_t;
extern const uint8_t u8g2_font_4x6_tf[];
extern const uint8_t u8g2_font_5x8_tf[];
extern const uint8_t u8g2_font_6x10_tf[];
extern const uint8_t u8g2_font_7x13_tf[];
extern const uint8_t u8g2_font_7x14B_tf[];
extern const uint8_t u8g2_font_helvB10_tf[];
extern const uint8_t u8g2_font_helvB12_tf[];
extern const uint8_t u8g2_font_logisoso16_tf[];
extern const uint8_t u8g2_font_logisoso32_tf[];

extern FILE* g_ops;

class U8G2_SH1106_128X64_NONAME_F_HW_I2C {
 public:
  U8G2_SH1106_128X64_NONAME_F_HW_I2C(int, uint8_t) {}
  bool begin() { return true; }
  void clearBuffer()  { if (g_ops) fprintf(g_ops, "FILLSCREEN 0000\n"); }
  void sendBuffer()   {}
  void setPowerSave(uint8_t) {}
  void setDrawColor(uint8_t c) { color_ = c; }
  void setFont(const uint8_t*) {}
  void drawStr(int x, int y, const char* s) {
    if (g_ops) fprintf(g_ops, "TEXT %d %d 1 %s %s %s\n", x, y - 8, color_ ? "FFFF" : "0000", "NONE", s);
  }
  void drawBox(int x, int y, int w, int h)   { rect(x, y, w, h, 1); }
  void drawFrame(int x, int y, int w, int h) { rect(x, y, w, h, 0); }
  void drawRBox(int x, int y, int w, int h, int r)   { rrect(x, y, w, h, r, 1); }
  void drawRFrame(int x, int y, int w, int h, int r) { rrect(x, y, w, h, r, 0); }
  void drawHLine(int x, int y, int w) { rect(x, y, w, 1, 1); }
 private:
  void rect(int x, int y, int w, int h, int fill) {
    if (g_ops) fprintf(g_ops, "RECT %d %d %d %d %s %d\n", x, y, w, h, color_ ? "FFFF" : "0000", fill);
  }
  void rrect(int x, int y, int w, int h, int r, int fill) {
    if (g_ops) fprintf(g_ops, "RRECT %d %d %d %d %d %s %d\n", x, y, w, h, r, color_ ? "FFFF" : "0000", fill);
  }
  uint8_t color_ = 1;
};
