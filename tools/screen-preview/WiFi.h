#pragma once
#include "Arduino.h"
// สตับ Wi-Fi ฝั่ง PC ใช้ได้ทั้งเฟิร์มแวร์ Station (STA) และ Host (SoftAP)
#define WIFI_OFF     0
#define WIFI_STA     1
#define WIFI_AP      2
#define WIFI_AP_STA  3
#define WL_CONNECTED 3
class IPAddressStub { public: String toString() const { return String("192.168.4.1"); } };
class WiFiClassStub {
public:
  void mode(int) {}
  void persistent(bool) {}
  void setSleep(bool) {}
  void disconnect(bool b = false) {}
  int  status() { return 0; }
  bool softAP(const char*, const char*, int ch = 1, int hidden = 0, int maxc = 4) { return true; }
  IPAddressStub softAPIP() { return IPAddressStub(); }
  int  softAPgetStationNum() { return 3; }
};
extern WiFiClassStub WiFi;
