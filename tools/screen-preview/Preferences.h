#pragma once
#include "Arduino.h"
class Preferences {
public:
  bool begin(const char*,bool=false){return true;}
  void end(){}
  int getInt(const char*,int d=0){return d;}
  unsigned char getUChar(const char*,unsigned char d=0){return d;}
  void putInt(const char*,int){}
  void putUChar(const char*,unsigned char){}
};
