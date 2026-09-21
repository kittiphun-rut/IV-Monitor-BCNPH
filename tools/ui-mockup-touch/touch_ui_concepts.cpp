/**
 * แบบร่าง UI แบบสมาร์ตโฟนสำหรับเครื่อง Host บนจอสัมผัส 2.4" (240x320 แนวตั้ง + ทัช XPT2046)
 * วาดด้วยคำสั่ง Adafruit_GFX ชุดเดียวกับเฟิร์มแวร์จริง เรนเดอร์เป็นภาพบน PC เพื่อเลือกแบบก่อนเขียนจริง
 *
 *  1 HOME      : ตารางการ์ดเตียงแบบไอคอนแอป แตะที่เตียงเพื่อเข้าดูรายละเอียด
 *  2 BED       : รายละเอียดเตียง + ปุ่มสั่งงาน (ถุงใหม่ / รับทราบ / ตั้งค่า)
 *  3 BED SET   : ตั้งค่าของเตียงด้วยปุ่ม - + บนจอ
 *  4 NUMPAD    : แป้นตัวเลขเมื่อแตะที่ค่าเพื่อพิมพ์ตรง ๆ
 *  5 SYSTEM    : ตั้งค่าระบบ (จำนวนเตียง ความสว่าง ปรับจอสัมผัส ข้อมูล Wi-Fi)
 *  6 ALARM     : จอเตือนเต็มจอ แตะที่ไหนก็ได้เพื่อพักเสียง
 */
#include "Arduino.h"
#include "Adafruit_ST7789.h"
#include "mockup_runtime.h"

#define C_BG        0x0000
#define C_CARD      0x18C5
#define C_CARD2     0x10A2
#define C_BAR       0x2104
#define C_TOPBAR    0x0A49
#define C_LINE      0x39E7
#define C_DIM       0x8410
#define C_WHITE     0xFFFF
#define C_GREEN     0x2FEB
#define C_CYAN      0x07FF
#define C_YELLOW    0xFFE0
#define C_ORANGE    0xFD20
#define C_RED       0xF800
#define C_SKY       0x5DFF
#define C_BTN       0x2945   // ปุ่มบนจอ

Adafruit_ST7789 tft = Adafruit_ST7789(nullptr, 0, 0, 0);
const int SCR_W = 240, SCR_H = 320;

enum BedStatus { S_NORMAL = 0, S_FAST, S_SLOW, S_NOFLOW, S_NEAREND, S_PAUSED, S_OFFLINE, S_EMPTY };

struct Bed {
  int id; const char* name; int rate; int target; int infused; int plan; BedStatus st;
};

uint16_t stColor(BedStatus s) {
  switch (s) {
    case S_NORMAL:  return C_GREEN;
    case S_FAST:    return C_ORANGE;
    case S_SLOW:    return C_YELLOW;
    case S_NOFLOW:  return C_RED;
    case S_NEAREND: return C_ORANGE;
    case S_PAUSED:  return C_SKY;
    default:        return C_LINE;
  }
}
const char* stWord(BedStatus s) {
  switch (s) {
    case S_NORMAL:  return "NORMAL";
    case S_FAST:    return "TOO FAST";
    case S_SLOW:    return "TOO SLOW";
    case S_NOFLOW:  return "NO FLOW";
    case S_NEAREND: return "NEXT BAG";
    case S_PAUSED:  return "PAUSED";
    case S_OFFLINE: return "OFFLINE";
    default:        return "-";
  }
}

// ---------------------------------------------------------------- ตัวช่วยวาด
void textAt(const String &s, int x, int y, uint8_t size, uint16_t col, uint16_t bg) {
  tft.setTextSize(size); tft.setTextColor(col, bg); tft.setCursor(x, y); tft.print(s);
}
void textRight(const String &s, int xr, int y, uint8_t size, uint16_t col, uint16_t bg) {
  textAt(s, xr - (int)s.length() * 6 * size, y, size, col, bg);
}
void textCenterIn(const String &s, int x, int w, int y, uint8_t size, uint16_t col, uint16_t bg) {
  textAt(s, x + (w - (int)s.length() * 6 * size) / 2, y, size, col, bg);
}
void bar(int x, int y, int w, int h, int pct, uint16_t fill) {
  if (pct < 0) pct = 0; if (pct > 100) pct = 100;
  tft.fillRoundRect(x, y, w, h, h / 2, C_BAR);
  int fw = w * pct / 100;
  if (fw >= h) tft.fillRoundRect(x, y, fw, h, h / 2, fill);
  else if (fw > 0) tft.fillRect(x, y, fw, h, fill);
}
// ปุ่มบนจอ (พื้นที่แตะอย่างน้อย 40x40 ตามแนวทาง UI สัมผัส)
void button(int x, int y, int w, int h, const String &label, uint16_t bg, uint16_t fg) {
  tft.fillRoundRect(x, y, w, h, 6, bg);
  textCenterIn(label, x, w, y + (h - 8) / 2, 1, fg, bg);
}
void buttonBig(int x, int y, int w, int h, const String &label, uint16_t bg, uint16_t fg) {
  tft.fillRoundRect(x, y, w, h, 8, bg);
  textCenterIn(label, x, w, y + (h - 16) / 2, 2, fg, bg);
}
void wifiIcon(int x, int y, int n) {
  int h[4] = {3, 6, 9, 12};
  for (int i = 0; i < 4; i++) tft.fillRect(x + i * 4, y + (12 - h[i]), 3, h[i], (n > i) ? C_GREEN : C_LINE);
}
void batteryIcon(int x, int y, int pct) {
  tft.drawRoundRect(x, y, 20, 10, 2, C_DIM);
  tft.fillRect(x + 20, y + 3, 2, 4, C_DIM);
  uint16_t c = (pct <= 20) ? C_RED : (pct <= 50 ? C_YELLOW : C_GREEN);
  int w = (16 * pct) / 100;
  if (w > 0) tft.fillRect(x + 2, y + 2, w, 6, c);
}

// ---------------------------------------------------------------- แถบสถานะและหัวเรื่อง
void statusBar(const char* clock, int wifiClients, int batPct) {
  tft.fillRect(0, 0, SCR_W, 22, C_TOPBAR);
  textAt(String(clock), 6, 7, 1, C_WHITE, C_TOPBAR);
  batteryIcon(SCR_W - 28, 6, batPct);
  wifiIcon(SCR_W - 50, 5, wifiClients);
}

// แถบหัวเรื่องพร้อมปุ่มย้อนกลับ (มุมซ้าย) และปุ่มตั้งค่า (มุมขวา) — พื้นที่แตะ 44x30
void titleBar(const String &title, bool back, bool gear) {
  tft.fillRect(0, 22, SCR_W, 34, C_CARD);
  if (back) {
    tft.fillRoundRect(4, 25, 40, 28, 5, C_BTN);
    textCenterIn("<", 4, 40, 35, 2, C_WHITE, C_BTN);
  }
  if (gear) {
    tft.fillRoundRect(SCR_W - 44, 25, 40, 28, 5, C_BTN);
    textCenterIn("=", SCR_W - 44, 40, 35, 2, C_WHITE, C_BTN);
  }
  textCenterIn(title, 48, SCR_W - 96, 32, 2, C_WHITE, C_CARD);
}

// ================================================================
// 1) HOME — การ์ดเตียงแบบไอคอนแอป (แตะการ์ด = เข้าดูเตียงนั้น)
// ================================================================
void drawBedTile(const Bed &b, int x, int y, int w, int h) {
  uint16_t sc = stColor(b.st);
  bool dim = (b.st == S_OFFLINE || b.st == S_EMPTY);
  tft.fillRoundRect(x, y, w, h, 7, dim ? C_CARD2 : C_CARD);
  tft.fillRoundRect(x, y, w, 18, 7, sc);
  tft.fillRect(x, y + 12, w, 6, sc);

  char bed[8];
  snprintf(bed, sizeof(bed), "B%02d", b.id);
  textAt(String(bed), x + 6, y + 5, 1, C_BG, sc);
  if (b.st != S_EMPTY) textRight(String(b.name), x + w - 6, y + 5, 1, C_BG, sc);

  if (b.st == S_EMPTY)   { textCenterIn("not used", x, w, y + 36, 1, C_DIM, C_CARD2); return; }
  if (b.st == S_OFFLINE) { textCenterIn("OFFLINE",  x, w, y + 36, 1, C_DIM, C_CARD2); return; }
  if (b.st == S_PAUSED)  { textCenterIn("PAUSED",   x, w, y + 32, 2, C_SKY, C_CARD); return; }

  textAt(String(b.rate), x + 8, y + 26, 3, (b.st == S_NOFLOW) ? sc : C_WHITE, C_CARD);
  textRight("mL/h", x + w - 6, y + 40, 1, C_DIM, C_CARD);
  int pct = (b.plan > 0) ? b.infused * 100 / b.plan : 0;
  bar(x + 6, y + h - 14, w - 12, 7, pct, (b.st == S_NEAREND) ? C_ORANGE : C_CYAN);
}

void drawHome(const Bed beds[8], int bedCount, const char* clock, bool alarm) {
  tft.fillScreen(C_BG);
  statusBar(clock, 3, 82);
  titleBar("SMART IV", false, true);

  for (int i = 0; i < 6 && i < bedCount; i++) {
    int col = i % 2, row = i / 2;
    drawBedTile(beds[i], 6 + col * 117, 62 + row * 78, 111, 72);
  }

  // แถบล่าง: สรุปเหตุการณ์ (แตะเพื่อกระโดดไปเตียงนั้น) + จุดบอกหน้า
  if (alarm) {
    tft.fillRect(0, 296, SCR_W, 24, C_RED);
    textCenterIn("! BED 02 NO FLOW - TAP", 0, SCR_W, 302, 1, C_WHITE, C_RED);
  } else {
    tft.fillRect(0, 296, SCR_W, 24, C_CARD);
    textCenterIn("ALL NORMAL   ONLINE 5/5", 0, SCR_W, 302, 1, C_GREEN, C_CARD);
  }
}

// ================================================================
// 2) BED — รายละเอียดเตียง + ปุ่มสั่งงาน
// ================================================================
void drawBedDetail(const Bed &b) {
  uint16_t sc = stColor(b.st);
  int pct = (b.plan > 0) ? b.infused * 100 / b.plan : 0;
  int mins = (b.rate > 5) ? (b.plan - b.infused) * 60 / b.rate : -1;

  tft.fillScreen(C_BG);
  statusBar("14:05", 3, 82);
  char title[16];
  snprintf(title, sizeof(title), "BED %02d", b.id);
  titleBar(String(title), true, false);

  tft.fillRoundRect(6, 62, 228, 108, 8, C_CARD);
  textAt(String(b.name), 14, 70, 1, C_DIM, C_CARD);
  tft.fillRoundRect(150, 66, 80, 20, 5, sc);
  textCenterIn(String(stWord(b.st)), 150, 80, 72, 1, C_BG, sc);

  textAt(String(b.rate), 14, 92, 6, (b.st == S_NOFLOW) ? sc : C_WHITE, C_CARD);
  textAt("mL/h", 14, 146, 1, C_DIM, C_CARD);
  textRight("SET " + String(b.target), 226, 146, 1, C_SKY, C_CARD);

  tft.fillRoundRect(6, 178, 228, 62, 8, C_CARD);
  textAt("INFUSED", 14, 186, 1, C_DIM, C_CARD);
  textRight(String(pct) + "%", 226, 186, 1, C_WHITE, C_CARD);
  bar(14, 200, 212, 12, pct, (b.st == S_NEAREND) ? C_ORANGE : C_CYAN);
  char sub[40];
  if (mins >= 0) snprintf(sub, sizeof(sub), "%d/%d mL   %dh %02dm left", b.infused, b.plan, mins / 60, mins % 60);
  else           snprintf(sub, sizeof(sub), "%d/%d mL   --h --m left", b.infused, b.plan);
  textAt(String(sub), 14, 220, 1, C_DIM, C_CARD);

  // ปุ่มสั่งงาน (สูง 44 px แตะง่าย)
  button(6,   250, 74, 44, "NEW BAG", C_BTN, C_WHITE);
  button(84,  250, 72, 44, "ACK",     C_BTN, C_WHITE);
  button(160, 250, 74, 44, "SETTINGS", C_BTN, C_WHITE);
  textCenterIn("tap value to edit", 0, SCR_W, 302, 1, C_DIM, C_BG);
}

// ================================================================
// 3) BED SETTINGS — ปรับค่าด้วยปุ่ม - + (แตะตัวเลขเพื่อพิมพ์)
// ================================================================
void drawSettingRow(int y, const char* label, const String &value, const char* unit) {
  tft.fillRoundRect(6, y, 228, 44, 7, C_CARD);
  textAt(String(label), 14, y + 6, 1, C_DIM, C_CARD);
  textAt(value, 14, y + 20, 2, C_WHITE, C_CARD);
  textAt(String(unit), 14 + (int)value.length() * 12 + 6, y + 26, 1, C_DIM, C_CARD);
  tft.fillRoundRect(140, y + 4, 44, 36, 6, C_BTN);
  textCenterIn("-", 140, 44, y + 14, 2, C_WHITE, C_BTN);
  tft.fillRoundRect(188, y + 4, 42, 36, 6, C_BTN);
  textCenterIn("+", 188, 42, y + 14, 2, C_WHITE, C_BTN);
}

void drawBedSettings(const Bed &b) {
  tft.fillScreen(C_BG);
  statusBar("14:05", 3, 82);
  char title[20];
  snprintf(title, sizeof(title), "SET B%02d", b.id);
  titleBar(String(title), true, false);

  drawSettingRow(62,  "TARGET RATE", String(b.target), "mL/h");
  drawSettingRow(112, "PLAN VOLUME", String(b.plan),   "mL");
  drawSettingRow(162, "DROP FACTOR", String(20),       "gtt/mL");
  drawSettingRow(212, "NEXT BAG AT", String(80),       "%");

  buttonBig(6,   266, 111, 46, "SAVE",   C_GREEN, C_BG);
  buttonBig(123, 266, 111, 46, "CANCEL", C_BTN,   C_WHITE);
}

// ================================================================
// 4) NUMPAD — แตะค่าเพื่อพิมพ์ตัวเลขโดยตรง
// ================================================================
void drawNumpad(const char* label, const char* value, const char* unit) {
  tft.fillScreen(C_BG);
  statusBar("14:05", 3, 82);
  titleBar("ENTER", true, false);

  tft.fillRoundRect(6, 62, 228, 46, 7, C_CARD);
  textAt(String(label), 14, 68, 1, C_DIM, C_CARD);
  textRight(String(unit), 226, 88, 1, C_DIM, C_CARD);
  textAt(String(value), 14, 82, 3, C_WHITE, C_CARD);

  const char* keys[12] = {"1","2","3","4","5","6","7","8","9","C","0","OK"};
  for (int i = 0; i < 12; i++) {
    int col = i % 3, row = i / 3;
    int x = 6 + col * 78, y = 116 + row * 50;
    bool ok = (i == 11), clr = (i == 9);
    tft.fillRoundRect(x, y, 72, 44, 7, ok ? C_GREEN : (clr ? C_ORANGE : C_BTN));
    textCenterIn(String(keys[i]), x, 72, y + 14, 2, (ok || clr) ? C_BG : C_WHITE, ok ? C_GREEN : (clr ? C_ORANGE : C_BTN));
  }
  textCenterIn("range 1 - 999", 0, SCR_W, 302, 1, C_DIM, C_BG);
}

// ================================================================
// 5) SYSTEM — ตั้งค่าระบบ
// ================================================================
void drawSystemMenu() {
  tft.fillScreen(C_BG);
  statusBar("14:05", 3, 82);
  titleBar("SETTINGS", true, false);

  drawSettingRow(62, "ACTIVE BEDS", String(5), "beds");

  const char* items[4] = {"SCREEN BRIGHTNESS", "TOUCH CALIBRATION", "WI-FI / WEB ACCESS", "ABOUT THIS DEVICE"};
  const char* subs[4]  = {"80%", "tap to run", "192.168.4.1", "Host v4.9.0-TOUCH"};
  for (int i = 0; i < 4; i++) {
    int y = 112 + i * 46;
    tft.fillRoundRect(6, y, 228, 40, 7, C_CARD);
    textAt(String(items[i]), 14, y + 8, 1, C_WHITE, C_CARD);
    textAt(String(subs[i]), 14, y + 24, 1, C_DIM, C_CARD);
    textRight(">", 224, y + 16, 2, C_DIM, C_CARD);
  }
  textCenterIn("แก้ค่าเหล่านี้จากหน้าเว็บได้เช่นกัน", 0, SCR_W, 302, 1, C_DIM, C_BG);
}

// ================================================================
// 6) ALARM — เต็มจอ แตะที่ไหนก็ได้เพื่อพักเสียง
// ================================================================
void drawAlarm(const Bed &b) {
  tft.fillScreen(C_RED);
  tft.fillRoundRect(10, 16, 220, 46, 8, C_WHITE);
  char bed[16];
  snprintf(bed, sizeof(bed), "BED %02d", b.id);
  textCenterIn(String(bed), 10, 220, 30, 3, C_RED, C_WHITE);

  textCenterIn(String(stWord(b.st)), 0, SCR_W, 82, 3, C_WHITE, C_RED);
  textCenterIn("CHECK IV LINE NOW", 0, SCR_W, 120, 1, C_YELLOW, C_RED);

  tft.fillRoundRect(16, 146, 208, 78, 8, C_BG);
  textAt("RATE", 28, 154, 1, C_DIM, C_BG);
  textAt(String(b.rate), 28, 170, 5, C_WHITE, C_BG);
  textRight("SET " + String(b.target), 212, 206, 1, C_SKY, C_BG);

  tft.fillRoundRect(16, 238, 208, 62, 10, C_WHITE);
  textCenterIn("TAP = SNOOZE 2 MIN", 16, 208, 258, 1, C_RED, C_WHITE);
  textCenterIn("BUTTON = SILENCE", 16, 208, 276, 1, C_RED, C_WHITE);
}

// ---------------------------------------------------------------- main
int main(int argc, char** argv) {
  openOps(argc > 1 ? argv[1] : "ops.txt");

  Bed beds[8] = {
    { 1, "SOMCHAI",  98, 100, 620, 1000, S_NORMAL },
    { 2, "MALEE",    96, 100, 310,  500, S_NORMAL },
    { 3, "PRASERT", 100, 100, 150, 1000, S_NORMAL },
    { 4, "WANIDA",    0, 100, 400,  500, S_PAUSED },
    { 5, "ARTHIT",   52,  50, 220,  500, S_NORMAL },
    { 6, "-",         0,   0,   0,    0, S_EMPTY },
    { 7, "-",         0,   0,   0,    0, S_EMPTY },
    { 8, "-",         0,   0,   0,    0, S_EMPTY },
  };
  Bed alertBeds[8] = {
    { 1, "SOMCHAI",  98, 100, 620, 1000, S_NORMAL },
    { 2, "MALEE",     0, 100, 310,  500, S_NOFLOW },
    { 3, "PRASERT", 134, 100, 150, 1000, S_FAST },
    { 4, "WANIDA",   96, 100, 430,  500, S_NEAREND },
    { 5, "ARTHIT",    0,  50, 220,  500, S_OFFLINE },
    { 6, "-",         0,   0,   0,    0, S_EMPTY },
    { 7, "-",         0,   0,   0,    0, S_EMPTY },
    { 8, "-",         0,   0,   0,    0, S_EMPTY },
  };

  drawHome(beds, 5, "14:05", false);        mark("t1_home");
  drawHome(alertBeds, 5, "16:42", true);    mark("t2_home_alert");
  drawBedDetail(beds[0]);                   mark("t3_bed");
  drawBedSettings(beds[0]);                 mark("t4_bed_settings");
  drawNumpad("TARGET RATE", "125", "mL/h"); mark("t5_numpad");
  drawSystemMenu();                         mark("t6_system");
  drawAlarm(alertBeds[1]);                  mark("t7_alarm");

  closeOps();
  return 0;
}
