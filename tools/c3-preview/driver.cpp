// ตัวขับสำหรับเรนเดอร์หน้าจอ OLED 0.42" (72x40) ของเฟิร์มแวร์ Station-C3-OLED บน PC
//
// ป้อน "สัญญาณหยดสังเคราะห์" เข้าตัวตรวจจับหยดตัวจริง (drop_detector.h) ไม่ใช่
// ตั้งค่าสถานะลัด ภาพที่ได้จึงสะท้อนพฤติกรรมจริงของอัลกอริทึม รวมถึงคำว่า
// LEARNING / READY ที่ขึ้นบนจอด้วย
#include "Arduino.h"
#include "WiFi.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include <cstdio>
#include <cmath>

FILE* g_ops = nullptr;
SerialClass Serial;
WiFiClassStub WiFi;
const uint8_t u8g2_font_4x6_tf[1]={0}, u8g2_font_5x8_tf[1]={0}, u8g2_font_6x10_tf[1]={0};
const uint8_t u8g2_font_7x13_tf[1]={0}, u8g2_font_7x14B_tf[1]={0}, u8g2_font_helvB10_tf[1]={0};
const uint8_t u8g2_font_helvB12_tf[1]={0}, u8g2_font_logisoso16_tf[1]={0}, u8g2_font_logisoso32_tf[1]={0};

// นาฬิกาหลักของการจำลอง เดินเป็นไมโครวินาที เพราะตัวตรวจจับสุ่มสัญญาณทุก 500 us
unsigned long g_micros = 0;
unsigned long millis(){ return g_micros / 1000UL; }
unsigned long micros(){ return g_micros; }
void delay(unsigned long ms){ g_micros += ms * 1000UL; }
void delayMicroseconds(unsigned long us){ g_micros += us; }

int g_adc = 2400;                       // ค่าที่ analogRead จะคืนให้ในรอบนี้
void pinMode(int,int){} void digitalWrite(int,int){} int digitalRead(int){return 1;}
int analogRead(int pin){ return pin == 0 ? 2450 : g_adc; }   // ขา 0 = แรงดันแบตเตอรี่
void analogReadResolution(int){} void analogSetAttenuation(adc_attenuation_t){}
void tone(int,unsigned int,unsigned long){} void noTone(int){}
void randomSeed(unsigned long){} long random(long m){return 0;} long random(long a,long){return a;}
void rgbLedWrite(int,int,int,int){} void neopixelWrite(int,int,int,int){}
long map(long x,long a,long b,long c,long d){return (x-a)*(d-c)/(b-a)+c;}
int esp_now_init(){return 0;}
int esp_now_register_recv_cb(esp_now_recv_cb_t){return 0;}
int esp_now_add_peer(const esp_now_peer_info_t*){return 0;}
int esp_now_del_peer(const uint8_t*){return 0;}
int esp_now_send(const uint8_t*,const uint8_t*,int){return 0;}
int esp_now_deinit(){return 0;}
int esp_wifi_set_channel(uint8_t,wifi_second_chan_t){return 0;}
int esp_wifi_get_channel(uint8_t* p,wifi_second_chan_t*){ *p=6; return 0; }
int esp_wifi_set_ps(int){return 0;}
bool getLocalTime(struct tm* info, unsigned long){
  time_t t = previewNow(); struct tm* g = localtime(&t); if (!g) return false; *info = *g; return true;
}
void emitMark(const char* n){ fprintf(g_ops, "MARK %s\n", n); }

#include "station_c3.cpp"   // สำเนาของ .ino ที่ run.sh คัดลอกมาให้

// ---------------------------------------------------------------------------
// เครื่องกำเนิดสัญญาณหยด — รูปคลื่นแบบเดียวกับที่ tools/detector-test ใช้
// หยดจริงทำให้แสงที่ตกถึงตัวรับลดลงชั่วขณะ ค่า ADC จึงลดลงเป็นพัลส์ลบ
// ---------------------------------------------------------------------------
static const int   BASE_ADC    = 2400;
static const int   PULSE_AMP   = 320;    // ความลึกของพัลส์
static const int   PULSE_MS    = 12;     // ความกว้างพัลส์
static const unsigned long STEP_US = 500;

// เดินเวลาไปข้างหน้า ms มิลลิวินาที โดยป้อนสัญญาณฐานเข้าตัวตรวจจับไปด้วย
void idleFor(unsigned long ms, bool count = true) {
  unsigned long until = g_micros + ms * 1000UL;
  while (g_micros < until) {
    g_adc = BASE_ADC;
    serviceDropSensor(count);
    g_micros += STEP_US;
  }
}

// ป้อนหยดหนึ่งหยด (พัลส์ครึ่งไซน์คว่ำ) แล้วรอจนครบช่วงเวลาระหว่างหยด
void feedOneDrop(unsigned long intervalMs, bool count = true) {
  unsigned long t0 = g_micros;
  while (g_micros - t0 < (unsigned long)PULSE_MS * 1000UL) {
    float ph = (float)(g_micros - t0) / (PULSE_MS * 1000.0f);
    g_adc = BASE_ADC - (int)(PULSE_AMP * sinf(ph * 3.14159265f));
    serviceDropSensor(count);
    g_micros += STEP_US;
  }
  if (intervalMs > (unsigned long)PULSE_MS) idleFor(intervalMs - PULSE_MS, count);
}

void feedDrops(int n, unsigned long intervalMs) {
  for (int i = 0; i < n; i++) feedOneDrop(intervalMs, true);
}

// ---------------------------------------------------------------------------
int main(int argc, char** argv) {
  g_ops = fopen(argc > 1 ? argv[1] : "ops.txt", "w");
  if (!g_ops) return 1;

  setenv("TZ", "ICT-7", 1); tzset();     // เวลาไทย เหมือนที่เฟิร์มแวร์ตั้งใน setup()
  g_micros = 3000UL * 1000UL;            // ให้พ้นช่วงบูตของเฟิร์มแวร์

  iv::DetectorConfig dcfg;
  detector.begin(dcfg);
  detector.reset(BASE_ADC);

  stationId     = 1;
  isRunning     = true;
  isClockSynced = true;
  dropFactor    = 20;
  nearEndPct    = 80;
  batteryVolts  = 3.95f;

  // ---- หน้าต้อนรับ ----
  drawSplash(); emitMark("00_splash");

  // ---- ป้อนหยดจริงจนตัวตรวจจับเรียนรู้เสร็จ (100 mL/h, drop factor 20 -> 3 วินาที/หยด) ----
  // ช่วงนี้จอจะขึ้นว่า LEARNING ตามพฤติกรรมจริงของอัลกอริทึม
  feedDrops(3, 1800);
  lastHostSyncTime = millis();
  targetRateHr  = 100.0f;
  totalPlanMl   = 1000.0f;
  totalDrops    = 10400;                 // ให้แถบความคืบหน้ามีค่าจริงให้ดู
  totalVolumeMl = 520.0f;
  currentPage = PAGE_RATE; uiMode = MODE_NORMAL;
  drawScreen(); emitMark("01_rate_learning");

  feedDrops(12, 1800);                   // เรียนรู้ครบ -> READY และอัตราไหลนิ่ง
  lastHostSyncTime = millis();
  drawScreen(); emitMark("02_rate_normal");

  // ---- จังหวะหยดกำลังตกกลางกระเปาะ (ภาพเดียวกันแต่คนละเฟส) ----
  idleFor(600); lastHostSyncTime = millis();
  drawScreen(); emitMark("03_rate_drip_mid");
  idleFor(600); lastHostSyncTime = millis();
  drawScreen(); emitMark("04_rate_drip_low");

  // ---- อัตราไหลต่ำ แสดงทศนิยมหนึ่งตำแหน่ง ----
  float keepRate = currentFlowRate_ml_hr;
  currentFlowRate_ml_hr = 9.5f;
  drawScreen(); emitMark("05_rate_low_value");
  currentFlowRate_ml_hr = keepRate;

  // ---- หน้าหยดต่อนาที ----
  currentPage = PAGE_GTT;
  drawScreen(); emitMark("06_gtt");

  // ---- หน้ายอดสะสม ----
  currentPage = PAGE_VOLUME;
  drawScreen(); emitMark("07_volume");

  // ---- หน้าสถานะเครื่อง ----
  currentPage = PAGE_STATUS;
  drawScreen(); emitMark("08_status");

  // ---- ลิงก์หลุด: หัวจอขึ้น NO LINK ----
  currentPage = PAGE_RATE;
  lastHostSyncTime = 0;
  drawScreen(); emitMark("09_no_link");
  lastHostSyncTime = millis();

  // ---- หยุดชั่วคราว ----
  isRunning = false;
  drawScreen(); emitMark("10_paused");
  isRunning = true;

  // ---- หน้าเตือนทั้งห้าแบบ (ครึ่งหนึ่งของรอบกะพริบ = พื้นขาว) ----
  uiMode = MODE_ALERT;
  struct { uint8_t code; const char* name; } alerts[] = {
    { ALERT_OCCLUSION, "11_alert_no_flow"  },
    { ALERT_TOO_FAST,  "12_alert_too_fast" },
    { ALERT_TOO_SLOW,  "13_alert_too_slow" },
    { ALERT_NEAR_END,  "14_alert_near_end" },
    { ALERT_COMPLETE,  "15_alert_finished" },
  };
  for (auto &a : alerts) {
    uiAlertCode = a.code;
    g_micros = 600000UL;                 // ครึ่งรอบที่พื้นกลับเป็นขาว
    drawScreen(); emitMark(a.name);
  }
  // แบบกลับสีของหน้าเดียวกัน เพื่อให้เห็นว่ากะพริบสลับกันอย่างไร
  uiAlertCode = ALERT_OCCLUSION;
  g_micros = 1200000UL;
  drawScreen(); emitMark("16_alert_blink_invert");

  // ---- หน้าตั้งเลขเตียง ----
  uiMode = MODE_SET_ID; stationId = 3;
  drawScreen(); emitMark("17_set_bed_id");
  stationId = 1;

  // ---- หน้าพักจอ ----
  uiMode = MODE_SAVER;
  currentFlowRate_ml_hr = 98.0f;
  drawScreen(); emitMark("18_screensaver");

  fclose(g_ops);
  return 0;
}
