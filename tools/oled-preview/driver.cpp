#include "Arduino.h"
#include "WiFi.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "ESPmDNS.h"
#include <cstdio>

FILE* g_ops = nullptr;
SerialClass Serial;
WiFiClassStub WiFi;
MDNSStub MDNS;
const uint8_t u8g2_font_4x6_tf[1]={0}, u8g2_font_5x8_tf[1]={0}, u8g2_font_6x10_tf[1]={0};
const uint8_t u8g2_font_7x13_tf[1]={0}, u8g2_font_7x14B_tf[1]={0}, u8g2_font_helvB10_tf[1]={0};
const uint8_t u8g2_font_helvB12_tf[1]={0}, u8g2_font_logisoso16_tf[1]={0}, u8g2_font_logisoso32_tf[1]={0};
unsigned long g_millis = 0;
unsigned long millis(){ return g_millis; }
unsigned long micros(){ return g_millis*1000UL; }
void delay(unsigned long ms){ g_millis += ms; }
void delayMicroseconds(unsigned long us){ g_millis += us/1000; }
void pinMode(int,int){} void digitalWrite(int,int){} int digitalRead(int){return 1;}
int analogRead(int){ return 2400; }
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

// ---- สตับของฟังก์ชันระบบที่เฟิร์มแวร์เรียกแต่ไม่เกี่ยวกับการวาดจอ ----
bool getLocalTime(struct tm* info, unsigned long){
  time_t t; time(&t); struct tm* g = localtime(&t); if (!g) return false; *info = *g; return true;
}
void rtc_gpio_pullup_en(int){} void rtc_gpio_pulldown_dis(int){} void rtc_gpio_deinit(int){}
int  esp_sleep_enable_ext0_wakeup(int,int){ return 0; }
void esp_deep_sleep_start(){}
esp_sleep_wakeup_cause_t esp_sleep_get_wakeup_cause(){ return (esp_sleep_wakeup_cause_t)0; }
int  esp_now_deinit(){ return 0; }
int  esp_wifi_set_ps(int){ return 0; }


#include "host_oled.cpp"   // สำเนาของ .ino ที่ run.sh คัดลอกมาให้

// ---- ใส่ข้อมูลจำลองให้เตียงหนึ่งเตียง ----
static void setBed(int i, bool online, bool running, float rate, float vol,
                   float plan, uint8_t alert, int8_t rssi, float batt) {
  StationData &s = stations[i];
  s.active = true;
  s.isRunning = running;
  s.rssi = rssi;
  s.batteryVolts = batt;
  s.flowRate_ml_hr = rate;
  s.totalVolumeMl = vol;
  s.totalDrops = (uint32_t)(vol * 20);
  s.msSinceLastDrop = running ? 1200 : 0;
  s.lastRecvTime = online ? g_millis : 0;
  s.firstRecvTime = online ? g_millis : 0;
  s.linkPct = online ? 100 : 0;
  s.rxTotal = online ? 1234 : 0;
  // เฟสเริ่มต้นของแอนิเมชัน: หยดล่าสุดเพิ่งตกไปเมื่อ msSinceLastDrop ที่แล้ว
  s.lastDropAtMs = (online && running && rate > 0.0f) ? (g_millis - s.msSinceLastDrop) : 0;
  s.alertCode = alert;
  s.cfg.targetRateHr = 100;
  s.cfg.planVolumeMl = plan;
  s.cfg.dropFactor = 20;
  s.nearEndPct = 80;
}

int main(int argc, char** argv) {
  g_ops = fopen(argc > 1 ? argv[1] : "ops.txt", "w");
  g_millis = 100000;

  // ---- หน้าเปิดเครื่อง ----
  u8g2.clearBuffer();
  u8g2.drawRFrame(0, 0, 128, 64, 4);
  u8g2.setFont(u8g2_font_7x14B_tf);
  u8g2.drawStr(14, 18, "SMART IV ALERT");
  u8g2.drawHLine(8, 23, 112);
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(14, 35, "Central Host Gateway");
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(14, 45, "BCN Phrae & MCU Collab");
  u8g2.drawStr(14, 55, "Host v" APP_VERSION " / Proto v3");
  emitMark("00_splash");

  // ---- 5 เตียง: ปกติ / ใกล้หมด / ไหลช้า / ไม่ไหล / เซนเซอร์ไม่จับหยด ----
  activeStationCount = 5;
  setBed(0, true,  true,  98.4f, 180.0f, 1000, ALERT_NONE,       -62, 3.95f);
  setBed(1, true,  true,  96.0f, 820.0f, 1000, ALERT_NEAR_END,   -65, 3.90f);
  setBed(2, true,  true,  48.0f, 410.0f, 1000, ALERT_TOO_SLOW,   -70, 3.85f);
  setBed(3, true,  true,   0.0f, 640.0f, 1000, ALERT_OCCLUSION,  -74, 3.80f);
  setBed(4, true,  true,   0.0f,   0.0f, 1000, ALERT_NO_SIGNAL,  -68, 3.88f);
  globalAlarmTriggered = true;

  currentOledPage = 0;
  updateHostOLED(); emitMark("01_block_5beds");

  // ---- หน้ารายละเอียดเตียง ----
  currentOledPage = 1; updateHostOLED(); emitMark("02_detail_bed1");
  currentOledPage = 5; updateHostOLED(); emitMark("03_detail_bed5_nosignal");

  // ---- ทุกเตียงปกติ: แถบล่างเป็นข้อมูลระบบ ----
  for (int i = 0; i < 5; i++) stations[i].alertCode = ALERT_NONE;
  globalAlarmTriggered = false;
  currentOledPage = 0; updateHostOLED(); emitMark("04_block_all_normal");

  // ---- แอนิเมชันหยด: จับภาพ 6 เฟรมติดกัน ห่างกันเฟรมละ 60 ms ----
  // ตั้งให้หยดล่าสุดของทุกเตียง "ตกพร้อมกันเดี๋ยวนี้" แล้วเดินเวลาไปทีละเฟรม
  for (int i = 0; i < 5; i++) {
    if (stations[i].flowRate_ml_hr > 0.0f) stations[i].lastDropAtMs = g_millis;
  }
  for (int f = 0; f < 6; f++) {
    char nm[32];
    snprintf(nm, sizeof(nm), "10_anim_grid_f%d", f);
    currentOledPage = 0; updateHostOLED(); emitMark(nm);
    g_millis += 60;
  }

  // ---- แอนิเมชันหยดบนหน้ารายละเอียดเตียง 1 ----
  stations[0].lastDropAtMs = g_millis;
  for (int f = 0; f < 6; f++) {
    char nm[32];
    snprintf(nm, sizeof(nm), "11_anim_detail_f%d", f);
    currentOledPage = 1; updateHostOLED(); emitMark(nm);
    g_millis += 60;
  }
  currentOledPage = 0;

  // ---- หน้าตรวจการเชื่อมต่อ + กรณีเลขเตียงซ้ำ ----
  for (int i = 0; i < 5; i++) {
    stations[i].macKnown = true;
    stations[i].srcMac[3] = 0x28; stations[i].srcMac[4] = 0xAB; stations[i].srcMac[5] = 0x10 + i;
    heardIdAt[i] = g_millis;
  }
  currentOledPage = activeStationCount + 1; updateHostOLED(); emitMark("13_link_diag");

  stations[0].idConflict = true;
  currentOledPage = activeStationCount + 1; updateHostOLED(); emitMark("14_link_diag_dup_id");
  currentOledPage = 0; updateHostOLED(); emitMark("15_block_dup_id_warning");
  stations[0].idConflict = false;

  heardIdAt[6] = g_millis;                 // ได้ยินเตียง 7 ทั้งที่เปิดใช้แค่ 5
  currentOledPage = activeStationCount + 1; updateHostOLED(); emitMark("16_link_diag_extra_bed");
  currentOledPage = 0; updateHostOLED(); emitMark("17_block_extra_bed_warning");
  heardIdAt[6] = 0;

  // ---- ลิงก์อ่อน: เตือนก่อนที่เตียงจะหลุดไปเป็น OFFLINE ----
  stations[0].linkPct = 40;
  currentOledPage = 1; updateHostOLED(); emitMark("12_detail_weak_link");
  stations[0].linkPct = 100;
  currentOledPage = 0;

  // ---- ใกล้หมด (ยังไม่รับทราบ) ----
  stations[1].alertCode = ALERT_NEAR_END;
  stations[1].nearEndAck = false;
  currentOledPage = 0; updateHostOLED(); emitMark("05_block_near_end");
  stations[1].alertCode = ALERT_NONE;

  // ---- 1 เตียง และ 2 เตียง (ผังต่างกัน) ----
  stations[0].lastDropAtMs = g_millis - 120;   // หยดอยู่กลางทางพอดี
  stations[1].lastDropAtMs = g_millis - 60;
  activeStationCount = 1; currentOledPage = 0; updateHostOLED(); emitMark("06_block_1bed");
  activeStationCount = 2; currentOledPage = 0; updateHostOLED(); emitMark("07_block_2beds");

  // ---- 8 เตียง ----
  activeStationCount = 8;
  for (int i = 5; i < 8; i++) setBed(i, true, true, 100.0f, 300.0f, 1000, ALERT_NONE, -66, 3.9f);
  for (int i = 0; i < 8; i++) {
    if (stations[i].flowRate_ml_hr > 0.0f) stations[i].lastDropAtMs = g_millis - 40 * i;  // เฟสต่างกันเล็กน้อย
  }
  stations[6].alertCode = ALERT_TOO_FAST;
  globalAlarmTriggered = true;
  currentOledPage = 0; updateHostOLED(); emitMark("08_block_8beds");

  // ---- Screensaver (นาฬิกา) ----
  globalAlarmTriggered = false;
  for (int i = 0; i < 8; i++) stations[i].alertCode = ALERT_NONE;
  oledScreensaverActive = true;
  updateHostOLED(); emitMark("09_screensaver");

  fclose(g_ops);
  return 0;
}
