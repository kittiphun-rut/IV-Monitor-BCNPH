// สตับ RTClib.h สำหรับพรีวิวหน้าจอบนเครื่อง PC (จำลอง DS3231 ที่ยังไม่ถูกตั้งเวลา)
#pragma once
#include <stdint.h>
#include <time.h>
#include "Wire.h"

class DateTime {
 public:
  DateTime() : _epoch(0) {}
  DateTime(uint32_t epoch) : _epoch(epoch) {}
  uint32_t unixtime() const { return _epoch; }
  uint16_t year()   const { return _tm().tm_year + 1900; }
  uint8_t  month()  const { return _tm().tm_mon + 1; }
  uint8_t  day()    const { return _tm().tm_mday; }
  uint8_t  hour()   const { return _tm().tm_hour; }
  uint8_t  minute() const { return _tm().tm_min; }
  uint8_t  second() const { return _tm().tm_sec; }
 private:
  struct tm _tm() const { time_t t = (time_t)_epoch; struct tm out; gmtime_r(&t, &out); return out; }
  uint32_t _epoch;
};

class RTC_DS3231 {
 public:
  bool begin(TwoWire* w = nullptr) { (void)w; return false; }   // พรีวิวถือว่าไม่มีโมดูล
  bool lostPower() { return true; }
  DateTime now() { return DateTime((uint32_t)0); }
  void adjust(const DateTime&) {}
  float getTemperature() { return 25.0f; }
};
