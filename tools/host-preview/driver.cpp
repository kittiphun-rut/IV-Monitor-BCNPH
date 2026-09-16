#include "Arduino.h"
#include "WiFi.h"
#include "ESPmDNS.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "driver/rtc_io.h"
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
void tone(int,unsigned int,unsigned long){} void noTone(int){}
long map(long x,long a,long b,long c,long d){return (x-a)*(d-c)/(b-a)+c;}
int esp_now_init(){return 0;} int esp_now_deinit(){return 0;}
int esp_now_register_recv_cb(esp_now_recv_cb_t){return 0;}
int esp_now_add_peer(const esp_now_peer_info_t*){return 0;}
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
                   uint8_t alert, bool running = true, bool online = true, bool ack = false) {
  StationData &s = stations[i];
  s.active = true;
  s.isRunning = running;
  s.rssi = -62;
  s.flowRate_ml_hr = rate;
  s.totalVolumeMl = infused;
  s.alertCode = alert;
  s.nearEndAck = ack;
  s.lastRecvTime = online ? g_millis : 0;
  s.cfg.targetRateHr = target;
  s.cfg.planVolumeMl = plan;
  s.cfg.dropFactor = 20;
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
  screensaverActive = false;
}

static void scenarioAlert() {
  activeStationCount = 5;
  setBed(0, "SOMCHAI",  98, 100, 620, 1000, ALERT_NONE);
  setBed(1, "MALEE",     0, 100, 310,  500, ALERT_OCCLUSION);
  setBed(2, "PRASERT", 134, 100, 150, 1000, ALERT_TOO_FAST);
  setBed(3, "WANIDA",   96, 100, 430,  500, ALERT_NEAR_END);
  setBed(4, "ARTHIT",    0,  50, 220,  500, ALERT_NONE, true, false);
  globalAlarmTriggered = true;
  screensaverActive = false;
}

static void renderMain(const char* markName) {
  uiMode = UI_MODE_MAIN;
  resetUiCaches();
  drawMainFramework();
  updateMainScreen();
  emitMark(markName);
}

int main(int argc, char** argv) {
  g_ops = fopen(argc > 1 ? argv[1] : "ops.txt", "w");
  setup();
  emitMark("splash");

  scenarioNormal();
  renderMain("main_normal");

  // เตือนใกล้หมดถุง (ไม่ใช่เหตุวิกฤต จึงยังอยู่หน้าหลัก)
  scenarioNormal();
  setBed(3, "WANIDA", 96, 100, 430, 500, ALERT_NEAR_END);
  renderMain("main_nearend");

  // มีเหตุวิกฤตแต่พักเสียงอยู่ -> หน้าหลักเน้นเตียงนั้น
  scenarioAlert();
  globalSnoozeUntil = g_millis + 60000;
  renderMain("main_alert");

  // เตียงครบ 8 เตียง -> แถวรายการด้านขวาย่อขนาดอัตโนมัติ
  scenarioNormal();
  activeStationCount = 8;
  setBed(5, "NARIN",    80,  80, 100,  500, ALERT_NONE);
  setBed(6, "SUDA",     60,  60, 480,  500, ALERT_NEAR_END);
  setBed(7, "KAMOL",     0,  75,   0,  500, ALERT_NONE, true, false);
  globalSnoozeUntil = 0;
  renderMain("main_8beds");

  // จอเตือนเต็มจอ
  scenarioAlert();
  globalSnoozeUntil = 0;
  uiMode = UI_MODE_ALARM;
  resetUiCaches();
  drawAlarmFramework();
  updateAlarmScreen();
  emitMark("alarm_fullscreen");

  // โหมดนาฬิกา
  scenarioNormal();
  uiMode = UI_MODE_SAVER;
  resetUiCaches();
  drawSaverFramework();
  updateSaverScreen();
  emitMark("screensaver");

  // กดปุ่มค้างเพื่อปิดเครื่อง
  uiMode = -1;
  drawPowerOffProgress(62);
  emitMark("poweroff");

  fclose(g_ops);
  return 0;
}
