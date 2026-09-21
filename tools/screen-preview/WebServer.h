#pragma once
#include "FS.h"
#include "Arduino.h"
#define HTTP_POST 2
#define HTTP_GET 1
class WebServer {
public:
  WebServer(int) {}
  void on(const char*, void(*)()) {}
  void on(const char*, int, void(*)()) {}
  void begin() {} void stop() {} void handleClient() {}
  bool authenticate(const char*, const char*) { return true; }
  void requestAuthentication() {}
  bool hasArg(const char*) { return true; }
  String arg(const char*) { return String("1"); }
  void send(int, const char*, const String&) {}
  void send(int, const char*, const char*) {}
  void send_P(int, const char*, const char*) {}
  void sendHeader(const char*, const String&) {}
  size_t streamFile(class fs::File&, const char*) { return 0; }
};
