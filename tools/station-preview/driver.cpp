#include "Arduino.h"
#include "WiFi.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include <cstdio>
FILE* g_ops = nullptr;
SerialClass Serial;
WiFiClassStub WiFi;
unsigned long g_millis = 0;
unsigned long millis(){ return g_millis; }
unsigned long micros(){ return g_millis*1000UL; }
void delay(unsigned long ms){ g_millis += ms; }
void delayMicroseconds(unsigned long us){ g_millis += us/1000; }
void pinMode(int,int){} void digitalWrite(int,int){} int digitalRead(int){return 1;}
int g_adc = 2731;
int analogRead(int){ return g_adc; }
void analogReadResolution(int){} void analogSetAttenuation(adc_attenuation_t){}
void tone(int,unsigned int,unsigned long){} void noTone(int){}
void randomSeed(unsigned long){} long random(long m){return 0;} long random(long a,long){return a;}
void rgbLedWrite(int,int,int,int){} void neopixelWrite(int,int,int,int){}
long map(long x,long a,long b,long c,long d){return (x-a)*(d-c)/(b-a)+c;}
int esp_now_init(){return 0;}
int esp_now_register_recv_cb(esp_now_recv_cb_t){return 0;}
int esp_now_add_peer(const esp_now_peer_info_t*){return 0;}
int esp_now_send(const uint8_t*,const uint8_t*,int){return 0;}
int esp_wifi_set_channel(uint8_t,wifi_second_chan_t){return 0;}
int esp_wifi_get_channel(uint8_t* p,wifi_second_chan_t*){ *p=1; return 0; }
void emitMark(const char* n){ fprintf(g_ops,"MARK %s\n",n); }
#include "station.cpp"   // สำเนาของ .ino ที่ run.sh คัดลอกมาให้
int main(int argc,char** argv){
  g_ops = fopen(argc>1?argv[1]:"ops.txt","w");
  setup();
  // ---- ใส่ข้อมูลตัวอย่างเสมือนได้รับจาก Host ----
  struct timeval tv = { (time_t)1789200300, 0 };   // 14:05 ตามเขตเวลาไทย
  settimeofday(&tv, NULL);
  isClockSynced = true;
  isHostOnline = true; lastHostRecvTime = g_millis; lastHostRssi = -62;
  targetRateHr = 100; totalPlanMl = 1000; dropFactor = 20; nearEndPct = 80;
  totalDrops = 620 * 20; totalVolumeMl = 620; currentFlowRate_ml_hr = 98; currentGttMin = 32.6f;
  hasFirstDropOccurred = true; lastDropTimestamp = g_millis;
  for (int i = 0; i < TREND_BARS; i++) {
    trendBin[i] = 88 + (i % 7) * 4 + ((i > 20) ? -14 : 0);
    if (i == 12 || i == 13) trendBin[i] = 40;
    trendHas[i] = true;
  }
  trendHead = 0; trendLive = 96;

  uiAlertCode = ALERT_NONE;
  stateNeedsRedraw = true; currentNursePage = 1;
  drawPage1Framework(); updatePage1Dynamic(); emitMark("p1_bag");
  stateNeedsRedraw = true; currentNursePage = 2;
  drawPage2Framework(); updatePage2Dynamic(); emitMark("p2_plan");
  stateNeedsRedraw = true; currentNursePage = 3;
  drawPage3Framework(); updatePage3Dynamic(); emitMark("p3_link");
  stateNeedsRedraw = true; currentNursePage = 4;
  drawPage4Framework(); updatePage4Dynamic(); emitMark("p4_trend");

  // ใกล้หมดถุง
  totalDrops = 840 * 20; totalVolumeMl = 840; uiAlertCode = ALERT_NEAR_END;
  stateNeedsRedraw = true; currentNursePage = 1;
  drawPage1Framework(); updatePage1Dynamic(); emitMark("p1_nearend");
  drawNearEndNoticeFramework(); updateNearEndNoticeDynamic(); emitMark("near_end");

  // เตือนสายพับ
  uiAlertCode = ALERT_OCCLUSION; currentFlowRate_ml_hr = 0;
  totalDrops = 620 * 20; totalVolumeMl = 620;
  stateNeedsRedraw = true; drawPage1Framework(); updatePage1Dynamic(); emitMark("p1_noflow");
  drawEmergencyScreen(ALERT_OCCLUSION); updateEmergencyDynamic(); emitMark("emergency");

  // หยุดชั่วคราว + screensaver + หน้าอื่น
  uiAlertCode = ALERT_NONE; isRunning = false;
  stateNeedsRedraw = true; drawPage1Framework(); updatePage1Dynamic(); emitMark("p1_paused");
  isRunning = true; currentFlowRate_ml_hr = 98;
  drawScreensaverFramework(); updateScreensaverDynamic(); emitMark("screensaver");
  drawDeveloperCreditScreen(); emitMark("credit");
  tempConfigStationId = 3; configAutoSaveTimeout = g_millis + 4000;
  drawStationIdConfigFramework(); updateStationIdConfigDynamic(); emitMark("config_id");
  drawHoldProgressHUD(3600); emitMark("hold_hud");
  fclose(g_ops);
  return 0;
}
