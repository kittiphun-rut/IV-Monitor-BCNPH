#pragma once
#include "Arduino.h"
#include <cstring>
// สตับ Wi-Fi ฝั่ง PC ใช้ได้ทั้งเฟิร์มแวร์ Station (STA) และ Host (SoftAP)
#define WIFI_OFF     0
#define WIFI_STA     1
#define WIFI_AP      2
#define WIFI_AP_STA  3
#define WL_CONNECTED 3
#define WL_DISCONNECTED 6
#define WIFI_SCAN_RUNNING (-1)
#define WIFI_SCAN_FAILED  (-2)
#define WIFI_AUTH_OPEN    0
class IPAddressStub {
  String v;
public:
  IPAddressStub(const char* a = "192.168.4.1") : v(a) {}
  String toString() const { return v; }
};
typedef IPAddressStub IPAddress;
class WiFiClassStub {
public:
  void mode(int) {}
  void persistent(bool) {}
  void setSleep(bool) {}
  void disconnect(bool wifioff = false, bool eraseap = false) {}
  void softAPdisconnect(bool wifioff = false) {}
  void begin(const char* ssid, const char* pass) {}
  IPAddressStub localIP() { return IPAddressStub("192.168.1.42"); }
  int  RSSI() { return -58; }
  // ---- การสแกนแบบไม่บล็อก ----
  int    scanNetworks(bool async = false) { return 2; }
  int    scanComplete() { return 2; }
  void   scanDelete() {}
  String SSID(int i) { return String(i == 0 ? "WARD-WIFI" : "HOSPITAL-GUEST"); }
  int    RSSI(int i) { return i == 0 ? -52 : -71; }
  int    encryptionType(int i) { return i == 0 ? 3 : WIFI_AUTH_OPEN; }
  int  status() { return 0; }
  bool softAP(const char*, const char*, int ch = 1, int hidden = 0, int maxc = 4) { return true; }
  IPAddressStub softAPIP() { return IPAddressStub(); }
  int  softAPgetStationNum() { return 3; }
  uint8_t* macAddress(uint8_t* mac) {
    static const uint8_t demo[6] = {0x24, 0x6F, 0x28, 0xAB, 0xCD, 0xEF};
    if (mac) memcpy(mac, demo, 6);
    return mac;
  }
  String macAddress() { return String("24:6F:28:AB:CD:EF"); }
};
extern WiFiClassStub WiFi;
