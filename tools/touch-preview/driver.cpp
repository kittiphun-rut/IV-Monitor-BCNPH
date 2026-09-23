#include "Arduino.h"
#include "WiFi.h"
#include "ESPmDNS.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "driver/rtc_io.h"
#include "XPT2046_Touchscreen.h"
bool XPT2046_Touchscreen::g_touched = false;
TS_Point XPT2046_Touchscreen::g_point;
#include <cstdio>

FILE* g_ops = nullptr;
SerialClass Serial; ESPClass ESP; WiFiClassStub WiFi; MDNSStub MDNS;
unsigned long g_millis = 1000;
unsigned long millis(){ return g_millis; }
unsigned long micros(){ return g_millis*1000UL; }
void delay(unsigned long ms){ g_millis += ms; }
void delayMicroseconds(unsigned long us){ g_millis += us/1000; }
void pinMode(int,int){} void digitalWrite(int,int){} int digitalRead(int){return 1;}
int analogRead(int){return 2400;} void analogReadResolution(int){} void analogSetAttenuation(adc_attenuation_t){}
void tone(int,unsigned int,unsigned long){}
bool ledcAttach(int,int,int){return true;}
void ledcWrite(int,int){}
void ledcSetup(int,int,int){}
void ledcAttachPin(int,int){} void noTone(int){}
long map(long x,long a,long b,long c,long d){return (x-a)*(d-c)/(b-a)+c;}
int esp_now_init(){return 0;} int esp_now_deinit(){return 0;}
int esp_now_register_recv_cb(esp_now_recv_cb_t){return 0;}
int esp_now_add_peer(const esp_now_peer_info_t*){return 0;}
int esp_now_del_peer(const uint8_t*){return 0;}
int esp_now_send(const uint8_t*,const uint8_t*,int){return 0;}
int esp_wifi_get_channel(uint8_t* p, wifi_second_chan_t*){ *p=1; return 0; }
int esp_wifi_set_channel(uint8_t, wifi_second_chan_t){return 0;}
int esp_wifi_set_ps(int){return 0;}
esp_sleep_wakeup_cause_t esp_sleep_get_wakeup_cause(){return ESP_SLEEP_WAKEUP_UNDEFINED;}
int esp_sleep_enable_ext0_wakeup(int,int){return 0;}
void esp_deep_sleep_start(){}
bool getLocalTime(struct tm* t, unsigned long){ time_t n=time(NULL); *t=*localtime(&n); return n>1600000000; }
void rtc_gpio_deinit(gpio_num_t){} void rtc_gpio_pullup_en(gpio_num_t){} void rtc_gpio_pulldown_dis(gpio_num_t){}
void emitMark(const char* n){ fprintf(g_ops,"MARK %s\n",n); }
#include "host.cpp"

// ---------------------------------------------------------------- ตัวขับสำหรับเรนเดอร์ภาพพรีวิว
static void setBed(int i, const char* name, float rate, float target, float infused, float plan,
                   uint8_t alert, bool running = true, bool online = true) {
  StationData &s = stations[i];
  s.active = true;
  s.isRunning = running;
  s.rssi = -62;
  s.flowRate_ml_hr = rate;
  s.totalVolumeMl = infused;
  s.alertCode = alert;
  s.lastRecvTime = online ? g_millis : 0;
  s.cfg.targetRateHr = target;
  s.cfg.planVolumeMl = plan;
  s.cfg.dropFactor = 20;
  // ค่าที่เพิ่มใน 4.9.2 — ต้องป้อนด้วย ไม่งั้นภาพตัวอย่างจะขึ้น LINK 0% และ MAC เป็นขีด
  s.linkPct = online ? 100 : 0;
  s.macKnown = online;
  s.srcMac[3] = 0x28; s.srcMac[4] = 0xAB; s.srcMac[5] = (uint8_t)(0x10 + i);
  s.totalDrops = (uint32_t)(infused * 20);
  s.firstRecvTime = online ? g_millis : 0;
  s.lastDropAtMs  = (online && running && rate > 0) ? (g_millis - 300) : 0;
  snprintf(s.cfg.patientName, sizeof(s.cfg.patientName), "%s", name);
}

static void scenarioNormal() {
  activeStationCount = 5;
  setBed(0, "SOMCHAI",  98, 100, 620, 1000, ALERT_NONE);
  setBed(1, "MALEE",    96, 100, 310,  500, ALERT_NONE);
  setBed(2, "PRASERT", 100, 100, 150, 1000, ALERT_NONE);
  setBed(3, "WANIDA",    0, 100, 400,  500, ALERT_NONE, false);
  setBed(4, "ARTHIT",   52,  50, 220,  500, ALERT_NONE);
  globalAlarmTriggered = false;
  globalSnoozeUntil = 0;
}

static void scenarioAlert() {
  activeStationCount = 5;
  setBed(0, "SOMCHAI",  98, 100, 620, 1000, ALERT_NONE);
  setBed(1, "MALEE",     0, 100, 310,  500, ALERT_OCCLUSION);
  setBed(2, "PRASERT", 134, 100, 150, 1000, ALERT_TOO_FAST);
  setBed(3, "WANIDA",   96, 100, 430,  500, ALERT_NEAR_END);
  setBed(4, "ARTHIT",    0,  50, 220,  500, ALERT_NONE, true, false);
  globalAlarmTriggered = true;
}

static void render(UiScreen scr, const char* markName) {
  uiScreen = scr;
  uiScreenDrawn = (UiScreen)-1;
  uiNeedFramework = true;
  updateHostDisplay();
  updateHostDisplay();          // รอบที่สองเพื่อให้ค่าที่วาดตามแคชขึ้นครบ
  emitMark(markName);
}

int main(int argc, char** argv) {
  g_ops = fopen(argc > 1 ? argv[1] : "ops.txt", "w");
  setup();
  emitMark("splash");

  scenarioNormal();
  render(SCR_HOME, "home_normal");

  scenarioNormal();
  setBed(3, "WANIDA", 96, 100, 430, 500, ALERT_NEAR_END);
  render(SCR_HOME, "home_nearend");

  scenarioAlert();
  globalSnoozeUntil = g_millis + 60000;      // พักเสียงอยู่ จึงยังเห็นหน้าหลัก
  render(SCR_HOME, "home_alert");

  scenarioNormal();
  activeStationCount = 8;
  setBed(5, "NARIN", 80, 80, 100, 500, ALERT_NONE);
  setBed(6, "SUDA",  60, 60, 480, 500, ALERT_NEAR_END);
  setBed(7, "KAMOL",  0, 75,   0, 500, ALERT_NONE, true, false);
  render(SCR_HOME, "home_8beds");

  scenarioNormal();
  uiSelectedBed = 0;
  render(SCR_BED, "bed_detail");

  editTargetRate = 100; editPlanVolume = 1000; editDropFactor = 20; editNearPct = 80;
  render(SCR_BED_SET, "bed_settings");

  uiEditField = 0;
  editNumLabel = "TARGET RATE"; editNumUnit = "mL/h"; editNumMax = 999;
  snprintf(editNumBuf, sizeof(editNumBuf), "125");
  render(SCR_NUMPAD, "numpad");

  screenBrightness = 100;
  render(SCR_SYS, "system");

  scenarioAlert();
  globalSnoozeUntil = 0;
  render(SCR_ALARM, "alarm");

  calibStep = 0;
  render(SCR_CALIB, "calibration");

  drawPowerOffProgress(62);
  emitMark("poweroff");

  fclose(g_ops);
  return 0;
}
