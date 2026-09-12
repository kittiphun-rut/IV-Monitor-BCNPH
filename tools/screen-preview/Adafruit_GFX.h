#pragma once
#include "Arduino.h"
#include <cstdio>
// จำลอง Adafruit_GFX โดยบันทึกคำสั่งวาดทั้งหมดเป็นบรรทัดข้อความ ให้สคริปต์ Python นำไปเรนเดอร์ภาพ
extern FILE* g_ops;
class Adafruit_GFX {
public:
  int cx=0, cy=0; uint8_t tsize=1; uint16_t tcolor=0xFFFF, tbg=0x0000;
  void fillScreen(uint16_t c){ fprintf(g_ops,"FILLSCREEN %u\n", c); }
  void fillRect(int x,int y,int w,int h,uint16_t c){ fprintf(g_ops,"RECT %d %d %d %d %u 1\n",x,y,w,h,c); }
  void drawRect(int x,int y,int w,int h,uint16_t c){ fprintf(g_ops,"RECT %d %d %d %d %u 0\n",x,y,w,h,c); }
  void fillRoundRect(int x,int y,int w,int h,int r,uint16_t c){ fprintf(g_ops,"RRECT %d %d %d %d %d %u 1\n",x,y,w,h,r,c); }
  void drawRoundRect(int x,int y,int w,int h,int r,uint16_t c){ fprintf(g_ops,"RRECT %d %d %d %d %d %u 0\n",x,y,w,h,r,c); }
  void drawFastHLine(int x,int y,int w,uint16_t c){ fprintf(g_ops,"RECT %d %d %d 1 %u 1\n",x,y,w,c); }
  void drawFastVLine(int x,int y,int h,uint16_t c){ fprintf(g_ops,"RECT %d %d 1 %d %u 1\n",x,y,h,c); }
  void drawLine(int x0,int y0,int x1,int y1,uint16_t c){ fprintf(g_ops,"LINE %d %d %d %d %u\n",x0,y0,x1,y1,c); }
  void drawPixel(int x,int y,uint16_t c){ fprintf(g_ops,"RECT %d %d 1 1 %u 1\n",x,y,c); }
  void fillCircle(int x,int y,int r,uint16_t c){ fprintf(g_ops,"CIRC %d %d %d %u\n",x,y,r,c); }
  void drawCircle(int x,int y,int r,uint16_t c){ fprintf(g_ops,"CIRCO %d %d %d %u\n",x,y,r,c); }
  void setTextSize(uint8_t s){ tsize = s ? s : 1; }
  void setTextColor(uint16_t c,uint16_t b){ tcolor=c; tbg=b; }
  void setTextWrap(bool){}
  void setCursor(int x,int y){ cx=x; cy=y; }
  void setRotation(uint8_t){}
  void print(const String& s){
    fprintf(g_ops,"TEXT %d %d %u %u %u %s\n", cx, cy, tsize, tcolor, tbg, s.c_str());
    cx += (int)s.length() * 6 * tsize;
  }
  void print(const char* s){ print(String(s)); }
  void getTextBounds(const String& s,int x,int y,int16_t* x1,int16_t* y1,uint16_t* w,uint16_t* h){
    *x1 = x; *y1 = y; *w = (uint16_t)(s.length()*6*tsize); *h = (uint16_t)(8*tsize);
  }
};
