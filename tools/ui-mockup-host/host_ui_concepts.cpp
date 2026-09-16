/**
 * แบบร่าง UI หน้าจอ TFT 2.8" (ST7789V 240x320) สำหรับเครื่อง Host — 3 แนวทางให้เลือก
 * วาดด้วยคำสั่ง Adafruit_GFX ชุดเดียวกับเฟิร์มแวร์จริง แต่เรนเดอร์เป็นภาพบน PC
 *
 *   A  BED GRID   (แนวนอน 320x240) การ์ดเตียง 8 ช่อง เห็นทุกเตียงพร้อมกันในหน้าเดียว
 *   B  FOCUS      (แนวนอน 320x240) เตียงที่ต้องดูตอนนี้ตัวใหญ่ครึ่งจอ + รายการเตียงอื่นด้านข้าง
 *   C  BIG LIST   (แนวตั้ง 240x320) รายการเตียงเรียงลงมา ตัวหนังสือใหญ่ อ่านง่ายที่สุด
 */
#include "Arduino.h"
#include "Adafruit_ST7789.h"
#include "mockup_runtime.h"

// ---------------------------------------------------------------- จานสี (ชุดเดียวกับเครื่องประจำเตียง)
#define C_BG        0x0000
#define C_CARD      0x18C5
#define C_CARD2     0x0A49
#define C_LINE      0x39E7
#define C_DIM       0x8410
#define C_WHITE     0xFFFF
#define C_GREEN     0x2FEB
#define C_CYAN      0x07FF
#define C_YELLOW    0xFFE0
#define C_ORANGE    0xFD20
#define C_RED       0xF800
#define C_PINK      0xFD79
#define C_SKY       0x5DFF
#define C_BAR       0x2104   // รางของแถบความคืบหน้า
#define C_OFFCARD   0x1082   // การ์ดเตียงที่ออฟไลน์/ยังไม่ใช้งาน

Adafruit_ST7789 tft = Adafruit_ST7789(nullptr, 0, 0, 0);
int SCR_W = 320, SCR_H = 240;

enum BedStatus { B_NORMAL = 0, B_FAST, B_SLOW, B_NOFLOW, B_NEAREND, B_DONE, B_PAUSED, B_OFFLINE, B_EMPTY };

struct Bed {
  int  id;
  const char* name;      // ชื่อย่อผู้ป่วย (ฟอนต์ GFX ไม่มีภาษาไทย จึงใช้อักษรโรมัน)
  int  rate;             // mL/h ที่วัดได้
  int  target;           // mL/h ตามแผน
  int  infused;          // mL ที่ให้ไปแล้ว
  int  plan;             // mL ตามแผน
  BedStatus st;
};

struct HostState {
  const char* clock;
  int  bedCount;
  int  onlineCount;
  int  wifiClients;
  int  hostBatPct;
  Bed  beds[8];
};

uint16_t stColor(BedStatus s) {
  switch (s) {
    case B_NORMAL:  return C_GREEN;
    case B_FAST:    return C_ORANGE;
    case B_SLOW:    return C_YELLOW;
    case B_NOFLOW:  return C_RED;
    case B_NEAREND: return C_ORANGE;
    case B_DONE:    return C_CYAN;
    case B_PAUSED:  return C_SKY;
    case B_OFFLINE: return C_LINE;
    default:        return C_LINE;
  }
}

const char* stWord(BedStatus s) {
  switch (s) {
    case B_NORMAL:  return "NORMAL";
    case B_FAST:    return "TOO FAST";
    case B_SLOW:    return "TOO SLOW";
    case B_NOFLOW:  return "NO FLOW";
    case B_NEAREND: return "NEXT BAG";
    case B_DONE:    return "COMPLETE";
    case B_PAUSED:  return "PAUSED";
    case B_OFFLINE: return "OFFLINE";
    default:        return "-";
  }
}

bool stIsAlarm(BedStatus s) {
  return s == B_FAST || s == B_SLOW || s == B_NOFLOW || s == B_DONE;
}

// ---------------------------------------------------------------- ตัวช่วยวาด
void textAt(const String &s, int x, int y, uint8_t size, uint16_t col, uint16_t bg) {
  tft.setTextSize(size);
  tft.setTextColor(col, bg);
  tft.setCursor(x, y);
  tft.print(s);
}

void textRight(const String &s, int xRight, int y, uint8_t size, uint16_t col, uint16_t bg) {
  textAt(s, xRight - (int)s.length() * 6 * size, y, size, col, bg);
}

void textCenterIn(const String &s, int x, int w, int y, uint8_t size, uint16_t col, uint16_t bg) {
  textAt(s, x + (w - (int)s.length() * 6 * size) / 2, y, size, col, bg);
}

void progressBar(int x, int y, int w, int h, int pct, uint16_t fill) {
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  tft.fillRoundRect(x, y, w, h, h / 2, C_BAR);
  int fw = w * pct / 100;
  if (fw >= h) tft.fillRoundRect(x, y, fw, h, h / 2, fill);
  else if (fw > 0) tft.fillRect(x, y, fw, h, fill);
}

void wifiIcon(int x, int y, int clients) {
  int h[4] = {3, 6, 9, 12};
  for (int i = 0; i < 4; i++) tft.fillRect(x + i * 4, y + (12 - h[i]), 3, h[i], (clients > i) ? C_GREEN : C_LINE);
}

void batteryIcon(int x, int y, int pct) {
  tft.drawRoundRect(x, y, 22, 11, 2, C_DIM);
  tft.fillRect(x + 22, y + 3, 2, 5, C_DIM);
  uint16_t c = (pct <= 20) ? C_RED : (pct <= 50 ? C_YELLOW : C_GREEN);
  int w = (18 * pct) / 100;
  if (w > 0) tft.fillRect(x + 2, y + 2, w, 7, c);
}

int pctOf(const Bed &b) { return (b.plan > 0) ? (b.infused * 100 / b.plan) : 0; }

// เตียงที่ควรถูกเน้น = เตียงที่มีเหตุก่อน แล้วค่อยเตียงใกล้หมด
int focusBed(const HostState &h) {
  for (int i = 0; i < h.bedCount; i++) if (stIsAlarm(h.beds[i].st)) return i;
  for (int i = 0; i < h.bedCount; i++) if (h.beds[i].st == B_NEAREND) return i;
  return 0;
}

// ---------------------------------------------------------------- แถบบน/ล่าง (ใช้ร่วมกัน)
void drawTopBar(const HostState &h, const char* title) {
  tft.fillRect(0, 0, SCR_W, 24, C_CARD2);
  textAt(String(SCR_W >= 300 ? title : "SMART IV"), 8, 8, 1, C_SKY, C_CARD2);
  textCenterIn(String(h.clock), 0, SCR_W, 6, 2, C_WHITE, C_CARD2);
  batteryIcon(SCR_W - 30, 7, h.hostBatPct);
  wifiIcon(SCR_W - 52, 6, h.wifiClients);
  tft.fillRect(0, 24, SCR_W, 2, C_LINE);
}

void drawBottomBar(const HostState &h) {
  int alarm = -1, near = -1;
  for (int i = 0; i < h.bedCount; i++) {
    if (alarm < 0 && stIsAlarm(h.beds[i].st)) alarm = i;
    if (near < 0 && h.beds[i].st == B_NEAREND) near = i;
  }
  int y = SCR_H - 18;
  if (alarm >= 0) {
    const Bed &b = h.beds[alarm];
    tft.fillRect(0, y, SCR_W, 18, C_RED);
    char buf[40];
    snprintf(buf, sizeof(buf), "! BED %02d  %s  -  CHECK NOW", b.id, stWord(b.st));
    textCenterIn(String(buf), 0, SCR_W, y + 5, 1, C_WHITE, C_RED);
  } else if (near >= 0) {
    tft.fillRect(0, y, SCR_W, 18, C_ORANGE);
    char buf[40];
    snprintf(buf, sizeof(buf), "BED %02d  PREPARE NEXT BAG", h.beds[near].id);
    textCenterIn(String(buf), 0, SCR_W, y + 5, 1, C_BG, C_ORANGE);
  } else {
    tft.fillRect(0, y, SCR_W, 18, C_CARD);
    char buf[40];
    snprintf(buf, sizeof(buf), "ALL NORMAL   ONLINE %d/%d", h.onlineCount, h.bedCount);
    textCenterIn(String(buf), 0, SCR_W, y + 5, 1, C_GREEN, C_CARD);
  }
}

// ================================================================
// แบบ A — BED GRID : เห็นทุกเตียงพร้อมกันในหน้าเดียว (แนวนอน 320x240)
// ================================================================
void drawBedCard(const Bed &b, int x, int y, int w, int h) {
  uint16_t sc = stColor(b.st);
  bool dim = (b.st == B_OFFLINE || b.st == B_EMPTY);

  tft.fillRoundRect(x, y, w, h, 5, dim ? C_OFFCARD : C_CARD);
  tft.fillRoundRect(x, y, w, 16, 5, sc);
  tft.fillRect(x, y + 11, w, 5, sc);
  char bed[8];
  snprintf(bed, sizeof(bed), "B%02d", b.id);
  textAt(String(bed), x + 5, y + 4, 1, C_BG, sc);
  if (b.st != B_EMPTY) textRight(String(b.name), x + w - 5, y + 4, 1, C_BG, sc);

  if (b.st == B_EMPTY) {
    textCenterIn("- - -", x, w, y + 44, 1, C_DIM, C_OFFCARD);
    return;
  }
  if (b.st == B_OFFLINE) {
    textCenterIn("OFFLINE", x, w, y + 40, 1, C_DIM, C_OFFCARD);
    return;
  }
  if (b.st == B_PAUSED) {
    textCenterIn("PAUSED", x, w, y + 34, 1, C_SKY, C_CARD);
    textCenterIn("hold", x, w, y + 50, 1, C_DIM, C_CARD);
    return;
  }

  textCenterIn(String(b.rate), x, w, y + 24, 3, stIsAlarm(b.st) ? sc : C_WHITE, C_CARD);
  textCenterIn("mL/h", x, w, y + 50, 1, C_DIM, C_CARD);
  progressBar(x + 6, y + 62, w - 12, 8, pctOf(b), (b.st == B_NEAREND) ? C_ORANGE : C_CYAN);
  if (b.st != B_NORMAL) {
    textCenterIn(String(stWord(b.st)), x, w, y + 76, 1, sc, C_CARD);      // มีเหตุ: โชว์คำเตือนอย่างเดียว
  } else {
    textAt(String(pctOf(b)) + "%", x + 6, y + 76, 1, C_DIM, C_CARD);      // ปกติ: %ที่ให้ไปแล้ว + อัตราเป้าหมาย
    textRight("/" + String(b.target), x + w - 6, y + 76, 1, C_DIM, C_CARD);
  }
}

void drawConceptA(const HostState &h) {
  SCR_W = 320; SCR_H = 240;
  tft.fillScreen(C_BG);
  drawTopBar(h, "SMART IV HOST");
  for (int i = 0; i < 8; i++) {
    int col = i % 4, row = i / 4;
    drawBedCard(h.beds[i], 5 + col * 78, 30 + row * 96, 74, 91);
  }
  drawBottomBar(h);
}

// ================================================================
// แบบ B — FOCUS : เตียงที่ต้องดูตอนนี้ตัวใหญ่ + รายการเตียงอื่นด้านข้าง
// ================================================================
void drawMiniRow(const Bed &b, int x, int y, int w, int h, bool isFocus) {
  uint16_t sc = stColor(b.st);
  tft.fillRoundRect(x, y, w, h, 4, isFocus ? C_CARD : C_OFFCARD);
  tft.fillRect(x, y, 4, h, sc);
  char bed[8];
  snprintf(bed, sizeof(bed), "B%02d", b.id);
  textAt(String(bed), x + 8, y + 6, 2, isFocus ? C_WHITE : C_DIM, isFocus ? C_CARD : C_OFFCARD);

  if (b.st == B_OFFLINE || b.st == B_EMPTY) {
    textRight(b.st == B_EMPTY ? "-" : "OFF", x + w - 6, y + 10, 1, C_DIM, isFocus ? C_CARD : C_OFFCARD);
  } else if (b.st == B_PAUSED) {
    textRight("||", x + w - 6, y + 8, 2, C_SKY, isFocus ? C_CARD : C_OFFCARD);
  } else {
    textRight(String(b.rate), x + w - 6, y + 6, 2, stIsAlarm(b.st) ? sc : C_WHITE, isFocus ? C_CARD : C_OFFCARD);
  }
  if (b.st != B_OFFLINE && b.st != B_EMPTY)
    progressBar(x + 8, y + h - 9, w - 16, 5, pctOf(b), (b.st == B_NEAREND) ? C_ORANGE : C_CYAN);
}

void drawConceptB(const HostState &h) {
  SCR_W = 320; SCR_H = 240;
  tft.fillScreen(C_BG);
  drawTopBar(h, "SMART IV HOST");

  const Bed &f = h.beds[focusBed(h)];
  uint16_t sc = stColor(f.st);

  tft.fillRoundRect(5, 30, 190, 188, 6, C_CARD);
  tft.fillRoundRect(5, 30, 190, 26, 6, sc);
  tft.fillRect(5, 48, 190, 8, sc);
  char bed[16];
  snprintf(bed, sizeof(bed), "BED %02d", f.id);
  textAt(String(bed), 12, 36, 2, C_BG, sc);
  textRight(String(f.name), 188, 38, 1, C_BG, sc);

  textAt(String(stWord(f.st)), 14, 66, 2, sc, C_CARD);
  textAt(String(f.rate), 14, 92, 6, stIsAlarm(f.st) ? sc : C_WHITE, C_CARD);
  textAt("mL/h", 14, 148, 1, C_DIM, C_CARD);
  textRight("SET " + String(f.target), 186, 148, 1, C_SKY, C_CARD);

  progressBar(14, 168, 172, 12, pctOf(f), (f.st == B_NEAREND) ? C_ORANGE : C_CYAN);
  // แถวสรุป: %ที่ให้ไปแล้ว | ปริมาณคงเหลือ | เวลาที่เหลือ (แบ่งพื้นที่ซ้าย-กลาง-ขวา ไม่ชนกัน)
  textAt(String(pctOf(f)) + "%", 14, 190, 1, C_DIM, C_CARD);
  textCenterIn(String(f.plan - f.infused) + " mL", 5, 190, 190, 1, C_WHITE, C_CARD);
  int minsLeft = (f.rate > 5) ? ((f.plan - f.infused) * 60 / f.rate) : -1;
  char tl[16];
  if (minsLeft >= 0) snprintf(tl, sizeof(tl), "%dh %02dm", minsLeft / 60, minsLeft % 60);
  else               snprintf(tl, sizeof(tl), "--h --m");
  textRight(String(tl), 186, 190, 1, stIsAlarm(f.st) ? sc : C_WHITE, C_CARD);

  int shown = 0;
  for (int i = 0; i < 8 && shown < 5; i++) {
    if (h.beds[i].st == B_EMPTY) continue;
    if (&h.beds[i] == &f) continue;
    drawMiniRow(h.beds[i], 200, 30 + shown * 38, 116, 34, false);
    shown++;
  }
  drawBottomBar(h);
}

// ================================================================
// แบบ C — BIG LIST : รายการเตียงเรียงลงมา ตัวหนังสือใหญ่ที่สุด (แนวตั้ง 240x320)
// ================================================================
void drawListRow(const Bed &b, int x, int y, int w, int h) {
  uint16_t sc = stColor(b.st);
  bool dim = (b.st == B_OFFLINE || b.st == B_EMPTY);
  tft.fillRoundRect(x, y, w, h, 5, dim ? C_OFFCARD : C_CARD);

  tft.fillRoundRect(x + 5, y + 5, 40, 40, 5, sc);
  char bed[8];
  snprintf(bed, sizeof(bed), "%02d", b.id);
  textCenterIn(String(bed), x + 5, 40, y + 17, 2, C_BG, sc);

  if (b.st == B_EMPTY) {
    textAt("- not used -", x + 54, y + 18, 1, C_DIM, C_OFFCARD);
    return;
  }
  textAt(String(b.name), x + 54, y + 8, 1, dim ? C_DIM : C_WHITE, dim ? C_OFFCARD : C_CARD);
  textAt(String(stWord(b.st)), x + 54, y + 22, 1, sc, dim ? C_OFFCARD : C_CARD);

  if (b.st == B_OFFLINE) return;
  if (b.st == B_PAUSED) {
    textRight("||", x + w - 8, y + 12, 3, C_SKY, C_CARD);
    return;
  }
  textRight(String(b.rate), x + w - 8, y + 6, 3, stIsAlarm(b.st) ? sc : C_WHITE, C_CARD);
  textRight("mL/h", x + w - 8, y + 32, 1, C_DIM, C_CARD);
  progressBar(x + 54, y + 36, 110, 6, pctOf(b), (b.st == B_NEAREND) ? C_ORANGE : C_CYAN);
}

void drawConceptC(const HostState &h) {
  SCR_W = 240; SCR_H = 320;
  tft.fillScreen(C_BG);
  drawTopBar(h, "SMART IV HOST");
  for (int i = 0; i < 5; i++) drawListRow(h.beds[i], 5, 30 + i * 54, 230, 50);
  drawBottomBar(h);
}

// ---------------------------------------------------------------- main
int main(int argc, char** argv) {
  openOps(argc > 1 ? argv[1] : "ops.txt");

  HostState normal = { "14:05", 5, 5, 3, 82, {
    { 1, "SOMCHAI",  98, 100, 620, 1000, B_NORMAL },
    { 2, "MALEE",    96, 100, 310,  500, B_NORMAL },
    { 3, "PRASERT", 100, 100, 150, 1000, B_NORMAL },
    { 4, "WANIDA",    0, 100, 400,  500, B_PAUSED },
    { 5, "ARTHIT",   52,  50, 220,  500, B_NORMAL },
    { 6, "-",         0,   0,   0,    0, B_EMPTY },
    { 7, "-",         0,   0,   0,    0, B_EMPTY },
    { 8, "-",         0,   0,   0,    0, B_EMPTY },
  }};

  HostState alert = { "16:42", 5, 4, 3, 64, {
    { 1, "SOMCHAI",  98, 100, 620, 1000, B_NORMAL },
    { 2, "MALEE",     0, 100, 310,  500, B_NOFLOW },
    { 3, "PRASERT", 134, 100, 150, 1000, B_FAST },
    { 4, "WANIDA",   96, 100, 430,  500, B_NEAREND },
    { 5, "ARTHIT",    0,  50, 220,  500, B_OFFLINE },
    { 6, "-",         0,   0,   0,    0, B_EMPTY },
    { 7, "-",         0,   0,   0,    0, B_EMPTY },
    { 8, "-",         0,   0,   0,    0, B_EMPTY },
  }};

  drawConceptA(normal); mark("A1_normal");
  drawConceptA(alert);  mark("A2_alert");
  drawConceptB(normal); mark("B1_normal");
  drawConceptB(alert);  mark("B2_alert");
  drawConceptC(normal); mark("C1_normal");
  drawConceptC(alert);  mark("C2_alert");

  closeOps();
  return 0;
}
