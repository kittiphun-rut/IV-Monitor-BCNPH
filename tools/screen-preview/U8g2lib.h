// สตับ U8g2lib.h สำหรับจำลองจอ OLED ขาวดำ 128x64 บนเครื่อง PC
// บันทึกทุกคำสั่งวาดลงไฟล์ ops แล้วให้ tools/oled-preview/render_oled.py เรนเดอร์เป็นภาพ
// U8g2 ใช้พิกัด y เป็น "เส้นฐานตัวอักษร" จึงบันทึกชื่อฟอนต์ไปด้วยเพื่อให้เรนเดอร์ได้ถูกตำแหน่ง
#pragma once
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include "Arduino.h"

#define U8G2_R0 0
#define U8X8_PIN_NONE 255

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
  void sendBuffer() {}
  void setPowerSave(uint8_t) {}
  void clearBuffer() { if (g_ops) fprintf(g_ops, "CLEAR\n"); }
  void setDrawColor(uint8_t c) { color_ = c; }

  void setFont(const uint8_t* f) {
    if      (f == u8g2_font_4x6_tf)        font_ = "4x6";
    else if (f == u8g2_font_5x8_tf)        font_ = "5x8";
    else if (f == u8g2_font_6x10_tf)       font_ = "6x10";
    else if (f == u8g2_font_7x13_tf)       font_ = "7x13";
    else if (f == u8g2_font_7x14B_tf)      font_ = "7x14B";
    else if (f == u8g2_font_helvB10_tf)    font_ = "helvB10";
    else if (f == u8g2_font_helvB12_tf)    font_ = "helvB12";
    else if (f == u8g2_font_logisoso16_tf) font_ = "logisoso16";
    else if (f == u8g2_font_logisoso32_tf) font_ = "logisoso32";
    else                                   font_ = "5x8";
  }

  void drawStr(int x, int y, const char* s) {
    if (g_ops) fprintf(g_ops, "TEXT %d %d %s %d %s\n", x, y, font_, color_, s);
  }
  void drawBox(int x, int y, int w, int h)              { emit("BOX",    x, y, w, h, 0); }
  void drawFrame(int x, int y, int w, int h)            { emit("FRAME",  x, y, w, h, 0); }
  void drawRBox(int x, int y, int w, int h, int r)      { emit("RBOX",   x, y, w, h, r); }
  void drawRFrame(int x, int y, int w, int h, int r)    { emit("RFRAME", x, y, w, h, r); }
  void drawHLine(int x, int y, int w)                   { emit("HLINE",  x, y, w, 1, 0); }

 private:
  void emit(const char* op, int x, int y, int w, int h, int r) {
    if (g_ops) fprintf(g_ops, "%s %d %d %d %d %d %d\n", op, x, y, w, h, r, color_);
  }
  const char* font_ = "5x8";
  uint8_t color_ = 1;
};
