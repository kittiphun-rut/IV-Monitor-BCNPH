#include "Arduino.h"
#include <cstdio>

FILE* g_ops = nullptr;
SerialClass Serial;
unsigned long g_millis = 0;
bool g_fakeButton = false;
int  g_pressCount = 0;
bool g_wizardCapture = false;
bool g_wizardStepMarked = false;

unsigned long millis(){ return g_millis; }
unsigned long micros(){ return g_millis * 1000UL; }
void emitMark(const char* name){ fprintf(g_ops, "MARK %s\n", name); }
void delay(unsigned long ms){
  if (g_wizardCapture) {
    if (!g_wizardStepMarked && ms <= 10) { emitMark("wizard_step"); g_wizardStepMarked = true; }
    if (ms >= 1500) emitMark("wizard_result");
  }
  g_millis += ms;
}
void delayMicroseconds(unsigned long us){ g_millis += us/1000; }
void pinMode(int,int){}
void digitalWrite(int,int){}
int digitalRead(int){
  if (!g_fakeButton) return HIGH;                 // ปกติ = ไม่กดปุ่ม
  bool pressed = ((g_millis / 200UL) % 8UL) == 7UL;   // จำลองกด 200 ms ทุก 1.6 s
  static bool last = false;
  if (pressed && !last) g_pressCount++;
  last = pressed;
  return pressed ? LOW : HIGH;
}
int analogRead(int){
  int base = (g_pressCount >= 2) ? 2790 : 1230;   // ขั้นที่ 1 = มีสารน้ำ, ขั้นที่ 2 = อากาศ
  return base + (int)(rand() % 13) - 6;
}
void analogReadResolution(int){}
void analogSetAttenuation(adc_attenuation_t){}
void tone(int,unsigned int){}
void noTone(int){}
void randomSeed(unsigned long s){ srand((unsigned)s); }
long random(long m){ return m > 0 ? (long)(rand() % m) : 0; }
long random(long a,long b){ return (b > a) ? a + (long)(rand() % (b - a)) : a; }
void rgbLedWrite(int,int,int,int){}
long map(long x,long a,long b,long c,long d){ return (x-a)*(d-c)/(b-a)+c; }

#include "sketch.cpp"

static void runFor(unsigned long ms, unsigned long step = 2) {
  unsigned long target = g_millis + ms;
  while (g_millis < target) { loop(); g_millis += step; }
}

int main(int argc, char** argv) {
  g_ops = fopen(argc > 1 ? argv[1] : "ops.txt", "w");
  setup();

  simRateIdx = 2;                 // โหมดจำลอง 60 mL/h เพื่อให้มีข้อมูลกราฟ
  simNextDropMs = 0;
  runFor(70000);                  // เดินเครื่อง 70 วินาที ให้กราฟแท่งเต็มหน้าต่างเวลา

  currentPage = PAGE_WAVE;  needRedraw = true; runFor(3000); emitMark("page1_wave");
  currentPage = PAGE_TREND; needRedraw = true; runFor(3000); emitMark("page2_trend");
  currentPage = PAGE_SENSOR;needRedraw = true; runFor(2000); emitMark("page3_sensor");

  currentState = STATE_ABOUT; needRedraw = true; runFor(200); emitMark("about");

  currentState = STATE_MAIN; currentPage = PAGE_WAVE; needRedraw = true; runFor(1500);
  drawHoldProgressHUD(3600); emitMark("hold_hud");

  // หน้าคาลิเบรต (ปิดโหมดจำลองก่อน แล้วใช้ปุ่มจำลอง)
  simRateIdx = 0;
  g_fakeButton = true; g_wizardCapture = true; g_pressCount = 0;
  executeCalibrationWizard();
  g_wizardCapture = false; g_fakeButton = false;

  fclose(g_ops);
  printf("ops written, virtual time %lu ms, drops %lu\n", g_millis, (unsigned long)totalDrops);
  return 0;
}
