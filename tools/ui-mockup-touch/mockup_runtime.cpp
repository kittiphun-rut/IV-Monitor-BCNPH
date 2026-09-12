#include "Arduino.h"
#include "mockup_runtime.h"
#include <cstdio>

FILE* g_ops = nullptr;
SerialClass Serial;
unsigned long millis() { return 0; }
unsigned long micros() { return 0; }
void delay(unsigned long) {}
void delayMicroseconds(unsigned long) {}
void pinMode(int, int) {}
void digitalWrite(int, int) {}
int digitalRead(int) { return 1; }
int analogRead(int) { return 0; }
void analogReadResolution(int) {}
void analogSetAttenuation(adc_attenuation_t) {}
void tone(int, unsigned int) {}
void noTone(int) {}
void randomSeed(unsigned long) {}
long random(long m) { return 0; }
long random(long a, long b) { return a; }
void rgbLedWrite(int, int, int, int) {}
long map(long x, long a, long b, long c, long d) { return (x - a) * (d - c) / (b - a) + c; }

void openOps(const char* path) { g_ops = fopen(path, "w"); }
void mark(const char* name) { fprintf(g_ops, "MARK %s\n", name); }
void closeOps() { fclose(g_ops); }
