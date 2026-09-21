// สตับ FS.h สำหรับพรีวิวหน้าจอบนเครื่อง PC (ไฟล์ทุกตัวถือว่าเปิดไม่สำเร็จ)
#pragma once
#include <stdint.h>
#include "Arduino.h"

#define FILE_READ   "r"
#define FILE_WRITE  "w"
#define FILE_APPEND "a"

namespace fs {

class File {
 public:
  operator bool() const { return false; }
  size_t print(const char*)      { return 0; }
  size_t print(const String&)    { return 0; }
  size_t print(char)             { return 0; }
  size_t print(int)              { return 0; }
  size_t print(unsigned int)     { return 0; }
  size_t print(long)             { return 0; }
  size_t print(unsigned long)    { return 0; }
  size_t print(float, int = 2)   { return 0; }
  size_t print(double, int = 2)  { return 0; }
  size_t println(const char* s = "") { (void)s; return 0; }
  size_t println(const String&)  { return 0; }
  size_t println(int)            { return 0; }
  size_t println(float, int = 2) { return 0; }
  size_t println(double, int = 2){ return 0; }
  size_t write(const uint8_t*, size_t n) { return n; }
  void close() {}
  void flush() {}
  const char* name() { return ""; }
  const char* path() { return ""; }
  size_t size() { return 0; }
  bool isDirectory() { return false; }
  File openNextFile(const char* mode = FILE_READ) { (void)mode; return File(); }
  int available() { return 0; }
  int read() { return -1; }
  size_t read(uint8_t*, size_t) { return 0; }
  void seek(uint32_t) {}
};

class FS {
 public:
  File open(const char* path, const char* mode = FILE_READ) { (void)path; (void)mode; return File(); }
  File open(const String& path, const char* mode = FILE_READ) { (void)path; (void)mode; return File(); }
  bool exists(const char* path) { (void)path; return false; }
  bool exists(const String& path) { (void)path; return false; }
  bool mkdir(const char* path) { (void)path; return false; }
  bool mkdir(const String& path) { (void)path; return false; }
  bool remove(const char* path) { (void)path; return false; }
  bool remove(const String& path) { (void)path; return false; }
};

}  // namespace fs

using fs::File;
