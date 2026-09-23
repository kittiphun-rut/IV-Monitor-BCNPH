#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <algorithm>
#define PI 3.14159265358979
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define INPUT_PULLDOWN 3
#define RISING 1
#define FALLING 2
#define CHANGE 3
#define IRAM_ATTR
#define PROGMEM
#define F(x) (x)
#include <sys/time.h>
typedef unsigned char byte;
typedef int esp_err_t;
typedef int portMUX_TYPE;
#define portMUX_INITIALIZER_UNLOCKED 0
#define portENTER_CRITICAL(x) ((void)0)
#define portEXIT_CRITICAL(x) ((void)0)
#define ESP_IDF_VERSION_VAL(a,b,c) ((a)*10000+(b)*100+(c))
#define ESP_IDF_VERSION ESP_IDF_VERSION_VAL(5,1,0)
#include <ctime>
bool getLocalTime(struct tm*, unsigned long ms=5000);
enum adc_attenuation_t { ADC_0db, ADC_2_5db, ADC_6db, ADC_11db };
class String {
public:
  std::string s;
  String() {}
  String(const char* c) : s(c?c:"") {}
  String(const std::string& v) : s(v) {}
  String(char c) { s = std::string(1,c); }
  String(int v) { char b[32]; snprintf(b,sizeof(b),"%d",v); s=b; }
  String(unsigned int v) { char b[32]; snprintf(b,sizeof(b),"%u",v); s=b; }
  String(long v) { char b[32]; snprintf(b,sizeof(b),"%ld",v); s=b; }
  String(unsigned long v) { char b[32]; snprintf(b,sizeof(b),"%lu",v); s=b; }
  String(float v, int d=2) { char b[48]; snprintf(b,sizeof(b),"%.*f",d,v); s=b; }
  String(double v, int d=2) { char b[48]; snprintf(b,sizeof(b),"%.*f",d,v); s=b; }
  unsigned int length() const { return s.size(); }
  void reserve(unsigned int n) { s.reserve(n); }
  const char* c_str() const { return s.c_str(); }
  int toInt() const { return atoi(s.c_str()); }
  String substring(unsigned int from, unsigned int to) const { return String(s.substr(from, to - from)); }
  String substring(unsigned int from) const { return String(s.substr(from)); }
  float toFloat() const { return (float)atof(s.c_str()); }
  void replace(const String& a, const String& b) {
    size_t p=0; while((p=s.find(a.s,p))!=std::string::npos){ s.replace(p,a.s.size(),b.s); p+=b.s.size(); }
  }
  String& operator+=(const String& o) { s += o.s; return *this; }
  String& operator+=(const char* c) { s += c; return *this; }
  String& operator+=(char c) { s += c; return *this; }
  bool operator==(const String& o) const { return s == o.s; }
  bool operator!=(const String& o) const { return s != o.s; }
};
inline String operator+(const String& a, const String& b) { return String(a.s + b.s); }
inline String operator+(const String& a, const char* b) { return String(a.s + b); }
inline String operator+(const char* a, const String& b) { return String(std::string(a) + b.s); }
unsigned long millis();
unsigned long micros();
void delay(unsigned long);
void delayMicroseconds(unsigned long);
void pinMode(int,int);
void digitalWrite(int,int);
int digitalRead(int);
int analogRead(int);
void analogReadResolution(int);
void analogSetAttenuation(adc_attenuation_t);
void tone(int,unsigned int,unsigned long d=0);
bool ledcAttach(int,int,int);
void ledcWrite(int,int);
void ledcSetup(int,int,int);
void ledcAttachPin(int,int);
void noTone(int);
// สตับของ interrupt: พรีวิวบน PC ไม่มีขาจริง จึงแค่เก็บ handler ไว้เฉย ๆ
inline int digitalPinToInterrupt(int pin) { return pin; }
inline void (*g_isrHandler)() = nullptr;
inline void attachInterrupt(int, void (*fn)(), int) { g_isrHandler = fn; }
inline void detachInterrupt(int) { g_isrHandler = nullptr; }
inline void noInterrupts() {}
inline void interrupts() {}
long map(long,long,long,long,long);
using std::min; using std::max;
class SerialClass {
public:
  void begin(unsigned long) {}
  void print(const char*) {} void print(const String&) {}
  void println(const char*) {} void println(const String&) {} void println() {}
  void printf(const char*,...) {}
};
extern SerialClass Serial;
class ESPClass { public: void restart() {} unsigned int getFreeHeap(){return 0;} };
extern ESPClass ESP;

// ---------------------------------------------------------------------------
// นาฬิกาของโหมดจำลอง — ตรึงไว้ให้ภาพที่เรนเดอร์ออกมาเหมือนเดิมทุกครั้งที่รัน
//
// ของเดิมอ่านนาฬิกาจริงของเครื่อง ภาพหน้าจอจึงเปลี่ยนทุกครั้งที่รันชุดตรวจ
// เพราะตัวเลขนาฬิกาบนจอขยับ ทำให้ git เห็นว่าไฟล์ภาพเปลี่ยนทั้งที่ไม่มีอะไรเปลี่ยนจริง
// และทำให้เทียบภาพ "ก่อนแก้" กับ "หลังแก้" เพื่อพิสูจน์ว่าโค้ดไม่เปลี่ยนพฤติกรรมไม่ได้เลย
//
// เปลี่ยนเวลาที่ตรึงได้ด้วยตัวแปรสภาพแวดล้อม PREVIEW_EPOCH
// ---------------------------------------------------------------------------
#include <cstdlib>
inline time_t previewNow() {
  const char* e = getenv("PREVIEW_EPOCH");
  if (e && *e) return (time_t)strtoll(e, nullptr, 10);
  return (time_t)1789000200;   // ตรึงไว้ที่เวลาหนึ่งซึ่งอ่านง่ายบนจอ
}
inline time_t preview_time(time_t* out) { time_t n = previewNow(); if (out) *out = n; return n; }
#define time(p) preview_time(p)
