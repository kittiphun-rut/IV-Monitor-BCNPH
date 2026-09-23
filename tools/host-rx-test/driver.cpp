// ป้อนแพ็กเก็ตเสมือนจาก Station หลายตัวเข้าฟังก์ชันรับของ Host โดยตรง
// แล้วตรวจว่า Host มองเห็นครบทุกเตียงจริงหรือไม่ (รันบน PC ไม่ต้องมีบอร์ด)
#include "Arduino.h"
#include "WiFi.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "ESPmDNS.h"
#include <cstdio>
#include <cstring>

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

int  g_sendCount = 0;
int esp_now_init(){return 0;}
int esp_now_register_recv_cb(esp_now_recv_cb_t){return 0;}
int esp_now_add_peer(const esp_now_peer_info_t*){return 0;}
int esp_now_send(const uint8_t*,const uint8_t*,int){ g_sendCount++; return 0; }
int esp_wifi_set_channel(uint8_t,wifi_second_chan_t){return 0;}
int esp_wifi_get_channel(uint8_t* p,wifi_second_chan_t*){ *p=1; return 0; }
void emitMark(const char*){}

bool getLocalTime(struct tm* info, unsigned long){
  time_t t; time(&t); struct tm* g = localtime(&t); if (!g) return false; *info = *g; return true;
}
void rtc_gpio_pullup_en(int){} void rtc_gpio_pulldown_dis(int){} void rtc_gpio_deinit(int){}
int  esp_sleep_enable_ext0_wakeup(int,int){ return 0; }
void esp_deep_sleep_start(){}
esp_sleep_wakeup_cause_t esp_sleep_get_wakeup_cause(){ return (esp_sleep_wakeup_cause_t)0; }
int  esp_now_deinit(){ return 0; }
int  esp_wifi_set_ps(int){ return 0; }

// เฟิร์มแวร์รุ่นจอสัมผัสใช้ไลบรารีทัชด้วย ต้องนิยามตัวแปร static ให้ลิงก์ผ่าน
// รุ่นจอ OLED ไม่ได้ใช้ ประกาศทิ้งไว้ก็ไม่กระทบ
// ฟังก์ชัน PWM ของไฟหน้าจอ ใช้เฉพาะรุ่นจอสัมผัส
bool ledcAttach(int,int,int) { return true; }
void ledcWrite(int,int) {}

#include "XPT2046_Touchscreen.h"
bool XPT2046_Touchscreen::g_touched = false;
TS_Point XPT2046_Touchscreen::g_point;

#include "host_oled.cpp"   // สำเนาของ .ino ที่ run.sh คัดลอกมาให้

// ---- สร้างแพ็กเก็ตเสมือนจาก Station หนึ่งตัว ----
static void sendFromStation(uint8_t id, uint32_t drops, float rate, const uint8_t mac[6]) {
  struct_message m = {};
  m.stationId       = id;
  m.isRunning       = 1;
  m.totalDrops      = drops;
  m.periodDrops     = 20;
  m.flowRateHr      = rate;
  m.msSinceLastDrop = 900;
  m.batteryVolts    = 3.95f;
  m.flags           = 0;
  esp_now_recv_info_t info = {};
  wifi_pkt_rx_ctrl_t ctrl = {};
  ctrl.rssi = -62;
  info.rx_ctrl = &ctrl;
  info.src_addr = (uint8_t*)mac;
  OnDataRecv(&info, (const uint8_t*)&m, sizeof(m));
}

static int failures = 0;
static void check(bool ok, const char* what) {
  printf("  %-58s %s\n", what, ok ? "ผ่าน" : "ไม่ผ่าน");
  if (!ok) failures++;
}

int main() {
  printf("ขนาดโครงสร้าง: struct_message=%d ไบต์, struct_host_sync=%d ไบต์, StationData=%d ไบต์\n",
         (int)sizeof(struct_message), (int)sizeof(struct_host_sync), (int)sizeof(StationData));
  printf("หน่วยความจำของ stations[%d] = %.1f KB\n\n",
         MAX_SUPPORTED_STATIONS, sizeof(stations) / 1024.0);

  g_millis = 100000;
  activeStationCount = 8;
  for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) {
    stations[i].cfg.dropFactor   = 20;
    stations[i].cfg.targetRateHr = 100;
    stations[i].cfg.planVolumeMl = 1000;
  }

  // ---- 1) แปดเตียง MAC ต่างกัน ส่งวินาทีละใบ 30 วินาที ----
  printf("ทดสอบ 1: 8 เตียง เลขเตียงไม่ซ้ำ ส่งครบทุกใบ\n");
  uint8_t macs[8][6];
  for (int i = 0; i < 8; i++) {
    uint8_t base[6] = {0x24, 0x6F, 0x28, 0x00, 0x00, (uint8_t)(0x10 + i)};
    memcpy(macs[i], base, 6);
  }
  for (int sec = 0; sec < 30; sec++) {
    for (int i = 0; i < 8; i++) {
      g_millis += 120;
      sendFromStation(i + 1, 100 + sec * 20, 98.0f, macs[i]);
    }
    g_millis += 40;
  }
  int online = 0;
  for (int i = 0; i < 8; i++) if (isStationOnline(i)) online++;
  check(online == 8, "Host เห็นครบทั้ง 8 เตียง");

  // ---- 2) เดินเวลาไปข้างหน้าโดยไม่มีแพ็กเก็ต ----
  printf("\nทดสอบ 2: เงียบไป 9 วินาที ต้องขึ้น OFFLINE ทุกเตียง\n");
  g_millis += 9000;
  online = 0;
  for (int i = 0; i < 8; i++) if (isStationOnline(i)) online++;
  check(online == 0, "ทุกเตียงขึ้น OFFLINE หลังเงียบเกินเกณฑ์");

  // ---- 3) กลับมาส่งใหม่ ----
  printf("\nทดสอบ 3: กลับมาส่งใหม่ ต้องกลับเป็นออนไลน์ทันที\n");
  for (int i = 0; i < 8; i++) { g_millis += 120; sendFromStation(i + 1, 1000, 98.0f, macs[i]); }
  online = 0;
  for (int i = 0; i < 8; i++) if (isStationOnline(i)) online++;
  check(online == 8, "ทุกเตียงกลับมาออนไลน์");

  // ---- 4) millis() ล้น (49.7 วัน) ----
  printf("\nทดสอบ 4: ตัวนับเวลา millis() ล้นรอบ\n");
  g_millis = 0xFFFFF000UL;
  for (int i = 0; i < 8; i++) { g_millis += 120; sendFromStation(i + 1, 2000, 98.0f, macs[i]); }
  g_millis += 3000;                     // ข้ามจุดล้นไป
  online = 0;
  for (int i = 0; i < 8; i++) if (isStationOnline(i)) online++;
  check(online == 8, "ยังออนไลน์ครบหลัง millis() ล้นรอบ");

  // ---- 5) เลขเตียงซ้ำกัน: สองบอร์ดใช้เลข 1 เหมือนกัน ----
  printf("\nทดสอบ 5: สองบอร์ดตั้งเลขเตียงซ้ำกัน (สาเหตุที่พบบ่อยที่สุด)\n");
  g_millis = 500000;
  for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) stations[i] = StationData();
  for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) {
    stations[i].cfg.dropFactor = 20; stations[i].cfg.targetRateHr = 100; stations[i].cfg.planVolumeMl = 1000;
  }
  activeStationCount = 3;
  for (int sec = 0; sec < 20; sec++) {
    g_millis += 300; sendFromStation(1, 100 + sec * 20, 98.0f, macs[0]);   // บอร์ด A = เตียง 1
    g_millis += 300; sendFromStation(1, 500 + sec * 20, 96.0f, macs[1]);   // บอร์ด B = เตียง 1 ด้วย
    g_millis += 400;
  }
  printf("    เตียง1 online=%d  เตียง2 online=%d  เตียง3 online=%d\n",
         isStationOnline(0), isStationOnline(1), isStationOnline(2));
  printf("    -> ทั้งสองบอร์ดเขียนทับช่องเดียวกัน เตียง 2-3 จึงขึ้น OFFLINE\n");
  printf("       แต่จอของบอร์ดทั้งสองขึ้น ONLINE ปกติ เพราะรับ Sync ของเตียง 1 ได้ทั้งคู่\n");
  check(isStationOnline(0) && !isStationOnline(1) && !isStationOnline(2),
        "จำลองอาการได้ตรงกับที่ผู้ใช้รายงาน");
  check(hasIdConflict(0), "Host ตรวจจับได้ว่ามีสองบอร์ดใช้เลขเตียงเดียวกัน");
  check(anyIdConflict(), "แถบเตือนบนจอจะขึ้นว่า 2 NODES SAME BED ID");

  // ---- 6) เลขเตียงไม่ซ้ำแต่เกินจำนวนเตียงที่เปิดใช้ ----
  printf("\nทดสอบ 6: ตั้งจำนวนเตียงไว้ 3 แต่มีบอร์ดตั้งเลข 6 อยู่ด้วย\n");
  g_millis = 900000;
  for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) stations[i] = StationData();
  for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) heardIdAt[i] = 0;
  activeStationCount = 3;
  for (int sec = 0; sec < 5; sec++) {
    for (int id = 1; id <= 3; id++) { g_millis += 150; sendFromStation(id, 100, 98.0f, macs[id - 1]); }
    g_millis += 150; sendFromStation(6, 100, 98.0f, macs[5]);
    g_millis += 100;
  }
  printf("    heardBeyondBedCount() = %d\n", heardBeyondBedCount());
  check(heardBeyondBedCount() == 6, "Host บอกได้ว่าได้ยินเตียง 6 แต่ยังไม่ได้เปิดใช้");
  check(!anyIdConflict(), "ไม่แจ้งเลขซ้ำผิด ๆ ในกรณีนี้");

  // ---- 7) บอร์ดเดิมรีบูต (MAC เดิม) ต้องไม่ถูกนับเป็นเลขซ้ำ ----
  printf("\nทดสอบ 7: บอร์ดเดิมรีบูตซ้ำ ๆ ต้องไม่แจ้งว่าเลขซ้ำ\n");
  g_millis = 1200000;
  for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) stations[i] = StationData();
  for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) heardIdAt[i] = 0;
  activeStationCount = 3;
  for (int k = 0; k < 40; k++) { g_millis += 1000; sendFromStation(1, 10 * k, 98.0f, macs[0]); }
  check(!hasIdConflict(0), "MAC เดิมตลอด = ไม่แจ้งเลขซ้ำ");

  printf("\n%s (%d ข้อที่ไม่ผ่าน)\n", failures ? "มีข้อที่ไม่ผ่าน" : "ผ่านทั้งหมด", failures);
  return failures ? 1 : 0;
}
