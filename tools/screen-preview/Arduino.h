#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <algorithm>
#define PI 3.1415926535897932384626433832795
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#include <ctime>
#include <sys/time.h>
typedef int esp_err_t;
typedef int portMUX_TYPE;
#define portMUX_INITIALIZER_UNLOCKED 0
#define portENTER_CRITICAL(x) ((void)0)
#define portEXIT_CRITICAL(x) ((void)0)
#define ESP_IDF_VERSION_VAL(a,b,c) ((a)*10000+(b)*100+(c))
#define ESP_IDF_VERSION ESP_IDF_VERSION_VAL(5,1,0)
void neopixelWrite(int,int,int,int);
enum adc_attenuation_t { ADC_0db, ADC_2_5db, ADC_6db, ADC_11db };
class String {
public:
  std::string s;
  String() {}
  String(const char* c) : s(c) {}
  String(const std::string& v) : s(v) {}
  String(int v) { char b[24]; snprintf(b,sizeof(b),"%d",v); s=b; }
  String(unsigned int v) { char b[24]; snprintf(b,sizeof(b),"%u",v); s=b; }
  String(long v) { char b[24]; snprintf(b,sizeof(b),"%ld",v); s=b; }
  String(unsigned long v) { char b[24]; snprintf(b,sizeof(b),"%lu",v); s=b; }
  String(float v, int d=-1) { char b[48]; if(d<0) snprintf(b,sizeof(b),"%g",v); else snprintf(b,sizeof(b),"%.*f",d,v); s=b; }
  unsigned int length() const { return s.size(); }
  void reserve(unsigned int n) { s.reserve(n); }
  const char* c_str() const { return s.c_str(); }
  String& operator+=(const String& o) { s += o.s; return *this; }
  String& operator+=(char c) { s += c; return *this; }
  String& operator+=(const char* c) { s += c; return *this; }
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
void tone(int,unsigned int);
void noTone(int);
void randomSeed(unsigned long);
long random(long);
long random(long,long);
void rgbLedWrite(int,int,int,int);
long map(long x, long a, long b, long c, long d);
using std::min; using std::max;
class SerialClass { public: void begin(unsigned long) {} void println(const char*) {} void print(const char*) {} void printf(const char*,...) {} };
extern SerialClass Serial;
// --- ตัวควบคุมการจำลองฝั่ง host ---
extern unsigned long g_millis;
extern bool g_fakeButton;
extern int  g_pressCount;
extern bool g_wizardCapture;
extern bool g_wizardStepMarked;
void emitMark(const char* name);
