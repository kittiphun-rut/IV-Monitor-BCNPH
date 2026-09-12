#pragma once
#include "Arduino.h"
#define WIFI_STA 1
class WiFiClassStub {
public:
  void mode(int) {}
  void disconnect(bool b=false) {}
  void setSleep(bool) {}
};
extern WiFiClassStub WiFi;
