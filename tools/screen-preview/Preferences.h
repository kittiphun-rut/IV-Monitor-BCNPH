#pragma once
#include "Arduino.h"
// สตับ Preferences (NVS) ฝั่ง PC — รวมทุกเมธอดที่เฟิร์มแวร์ทั้งสามตัวเรียกใช้
class Preferences {
public:
  bool begin(const char*, bool = false) { return true; }
  void end() {} void clear() {}
  bool isKey(const char*) { return false; }
  int           getInt(const char* k, int d = 0) { return d; }
  unsigned int  getUInt(const char*, unsigned int d = 0) { return d; }
  unsigned char getUChar(const char*, unsigned char d = 0) { return d; }
  String        getString(const char*, const char* d = "") { return String(d); }
  size_t        getBytesLength(const char*) { return 0; }
  size_t        getBytes(const char*, void*, size_t) { return 0; }
  void putInt(const char*, int) {}
  void putUInt(const char*, unsigned int) {}
  void putUChar(const char*, unsigned char) {}
  void putString(const char*, const String&) {}
  void putBytes(const char*, const void*, size_t) {}
};
