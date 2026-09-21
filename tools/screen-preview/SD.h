// สตับ SD.h สำหรับพรีวิวหน้าจอบนเครื่อง PC (ถือว่าไม่มีการ์ดเสียบอยู่)
#pragma once
#include "FS.h"
#include "SPI.h"

class SDFS : public fs::FS {
 public:
  bool begin(uint8_t csPin = 0, SPIClass& spi = SPI, uint32_t freq = 4000000,
             const char* mount = "/sd", uint8_t maxFiles = 5) {
    (void)csPin; (void)spi; (void)freq; (void)mount; (void)maxFiles; return false;
  }
  void end() {}
  uint64_t cardSize()  { return 0; }
  uint64_t totalBytes() { return 0; }
  uint64_t usedBytes()  { return 0; }
};

inline SDFS SD;
