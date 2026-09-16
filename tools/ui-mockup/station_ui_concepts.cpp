/**
 * แบบร่าง UI ของจอ TFT 172x320 ฝั่ง Bed Station — 3 แนวทางให้เลือก
 * ใช้ Adafruit_GFX API จำลองจาก tools/screen-preview เพื่อเรนเดอร์เป็นภาพ PNG บน PC
 * โค้ดในไฟล์นี้เขียนด้วยคำสั่งวาดชุดเดียวกับเฟิร์มแวร์จริง จึงยกไปใช้ต่อได้ทันทีเมื่อเลือกแบบได้แล้ว
 */
#include "Arduino.h"
#include "Adafruit_ST7789.h"
#include "mockup_runtime.h"

// ---------------------------------------------------------------- จานสี
#define C_BG        0x0000
#define C_CARD      0x18C5   // เทาเข้ม
#define C_CARD2     0x0A49   // น้ำเงินเข้มมาก
#define C_LINE      0x39E7
#define C_DIM       0x8410   // เทาอ่อน (ข้อความรอง)
#define C_WHITE     0xFFFF
#define C_GREEN     0x2FEB
#define C_CYAN      0x07FF
#define C_YELLOW    0xFFE0
#define C_ORANGE    0xFD20
#define C_RED       0xF800
#define C_PINK      0xFD79
#define C_SKY       0x5DFF
#define C_NAVY      0x09CD

Adafruit_ST7789 tft = Adafruit_ST7789(nullptr, 0, 0, 0);

enum Status { ST_NORMAL = 0, ST_FAST, ST_SLOW, ST_NOFLOW, ST_NEAREND, ST_PAUSED };

struct UiState {
  int  bed;
  const char* clock;
  int  rate;        // mL/h ที่วัดได้
  int  target;      // mL/h ตามแผน
  int  infused;     // mL ที่ให้ไปแล้ว
  int  plan;        // mL ตามแผน
  const char* endClock;
  int  minsLeft;
  Status st;
  bool hostOnline;
};

uint16_t statusColor(Status s) {
  switch (s) {
    case ST_NORMAL:  return C_GREEN;
    case ST_FAST:    return C_ORANGE;
    case ST_SLOW:    return C_YELLOW;
    case ST_NOFLOW:  return C_RED;
    case ST_NEAREND: return C_ORANGE;
    default:         return C_SKY;
  }
}

const char* statusWord(Status s) {
  switch (s) {
    case ST_NORMAL:  return "NORMAL";
    case ST_FAST:    return "TOO FAST";
    case ST_SLOW:    return "TOO SLOW";
    case ST_NOFLOW:  return "NO FLOW";
    case ST_NEAREND: return "NEXT BAG";
    default:         return "PAUSED";
  }
}

// ---------------------------------------------------------------- ตัวช่วยวาด
void textAt(const String &s, int x, int y, uint8_t size, uint16_t col, uint16_t bg) {
  tft.setTextSize(size);
  tft.setTextColor(col, bg);
  tft.setCursor(x, y);
  tft.print(s);
}

void textCenter(const String &s, int y, uint8_t size, uint16_t col, uint16_t bg) {
  int w = (int)s.length() * 6 * size;
  textAt(s, (172 - w) / 2, y, size, col, bg);
}

void textCenterIn(const String &s, int x, int w, int y, uint8_t size, uint16_t col, uint16_t bg) {
  int tw = (int)s.length() * 6 * size;
  textAt(s, x + (w - tw) / 2, y, size, col, bg);
}

void textRight(const String &s, int xRight, int y, uint8_t size, uint16_t col, uint16_t bg) {
  textAt(s, xRight - (int)s.length() * 6 * size, y, size, col, bg);
}

// แถบสัญญาณ Host แบบเล็ก
void drawLinkBars(int x, int y, bool online) {
  int h[4] = {4, 7, 10, 13};
  for (int i = 0; i < 4; i++)
    tft.fillRect(x + i * 4, y + (13 - h[i]), 3, h[i], online ? C_GREEN : C_LINE);
  if (!online) {
    tft.drawLine(x, y, x + 14, y + 13, C_RED);
    tft.drawLine(x, y + 13, x + 14, y, C_RED);
  }
}

// แถบความคืบหน้าแบบโค้ง
void progressBar(int x, int y, int w, int h, int pct, uint16_t fill, uint16_t track) {
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  tft.fillRoundRect(x, y, w, h, h / 2, track);
  int fw = w * pct / 100;
  if (fw > h) tft.fillRoundRect(x, y, fw, h, h / 2, fill);
  else if (fw > 0) tft.fillRect(x, y, fw, h, fill);
}

// เกจเปรียบเทียบอัตราจริงกับเป้าหมาย: กึ่งกลาง = ตรงเป้า, ซ้าย = ช้ากว่า, ขวา = เร็วกว่า
void deviationGauge(int x, int y, int w, int h, int rate, int target, uint16_t mark) {
  tft.fillRoundRect(x, y, w, h, 3, C_CARD2);
  int mid = x + w / 2;
  // โซนยอมรับได้ +/-10%
  tft.fillRect(mid - w / 10, y, w / 5, h, 0x1B0C);
  for (int i = 1; i < 4; i++) tft.drawFastVLine(x + w * i / 4, y + h - 3, 3, C_LINE);
  tft.drawFastVLine(mid, y, h, C_DIM);
  float dev = (target > 0) ? ((float)(rate - target) / (float)target) : 0.0f;
  if (dev < -0.5f) dev = -0.5f;
  if (dev > 0.5f) dev = 0.5f;
  int px = mid + (int)(dev * w);
  if (px < x + 2) px = x + 2;
  if (px > x + w - 4) px = x + w - 4;
  tft.fillRect(px - 1, y - 2, 4, h + 4, mark);
}

String timeLeftStr(const UiState &u, bool compact) {
  if (u.minsLeft <= 0 || u.st == ST_NOFLOW || u.st == ST_PAUSED) return compact ? String("--h--m") : String("--h --m");
  char b[20];
  snprintf(b, sizeof(b), compact ? "%dh%02dm" : "%dh %02dm", u.minsLeft / 60, u.minsLeft % 60);
  return String(b);
}

// ---------------------------------------------------------------- แถบบนสุด (ใช้ร่วมทุกแบบ)
void drawTopBar(const UiState &u) {
  tft.fillRect(0, 0, 172, 22, C_CARD2);
  tft.fillRect(0, 22, 172, 2, statusColor(u.st));

  char bed[8];
  snprintf(bed, sizeof(bed), "B%02d", u.bed);
  tft.fillRoundRect(4, 3, 30, 16, 4, statusColor(u.st));
  textCenterIn(String(bed), 4, 30, 7, 1, C_BG, statusColor(u.st));

  textCenter(String(u.clock), 7, 1, C_WHITE, C_CARD2);
  drawLinkBars(150, 4, u.hostOnline);
}

void drawPageDots(int activeIdx, int count, int y) {
  int w = count * 12 - 6;
  int x = (172 - w) / 2;
  for (int i = 0; i < count; i++) {
    if (i == activeIdx) tft.fillRoundRect(x + i * 12, y, 8, 4, 2, C_SKY);
    else                tft.fillRoundRect(x + i * 12, y, 6, 4, 2, C_LINE);
  }
}

// ================================================================
// แบบ A — BIG NUMBER : เน้นตัวเลขอัตราไหลใหญ่ที่สุด อ่านได้จากปลายเตียง
// ================================================================
void drawConceptA(const UiState &u) {
  tft.fillScreen(C_BG);
  drawTopBar(u);

  textCenter("FLOW RATE", 34, 1, C_DIM, C_BG);
  char rateBuf[8];
  snprintf(rateBuf, sizeof(rateBuf), "%d", u.rate);
  uint16_t rateCol = (u.st == ST_NOFLOW) ? C_RED : (u.st == ST_PAUSED ? C_SKY : C_WHITE);
  if (u.st == ST_PAUSED) textCenter("PAUSED", 60, 5, C_SKY, C_BG);
  else                   textCenter(String(rateBuf), 48, 7, rateCol, C_BG);
  textCenter("mL/h", 112, 2, C_DIM, C_BG);

  // เทียบกับเป้าหมาย
  deviationGauge(16, 136, 140, 14, u.rate, u.target, statusColor(u.st));
  textCenter("TARGET " + String(u.target), 156, 1, C_DIM, C_BG);

  // ความคืบหน้าของถุง
  int pct = (u.plan > 0) ? (u.infused * 100 / u.plan) : 0;
  textAt("INFUSED", 16, 176, 1, C_DIM, C_BG);
  textRight(String(pct) + "%", 156, 172, 2, C_WHITE, C_BG);
  progressBar(16, 192, 140, 12, pct, (u.st == ST_NEAREND) ? C_ORANGE : C_CYAN, C_CARD);

  // เวลาที่เหลือ
  tft.fillRoundRect(6, 214, 160, 60, 8, C_CARD);
  textCenter("TIME LEFT", 220, 1, C_DIM, C_CARD);
  textCenter(timeLeftStr(u, false), 232, 3, C_WHITE, C_CARD);
  textCenter("END " + String(u.endClock) + "   LEFT " + String(u.plan - u.infused) + " mL", 258, 1, C_DIM, C_CARD);

  // สถานะ
  tft.fillRoundRect(6, 280, 160, 26, 6, statusColor(u.st));
  textCenter(String(statusWord(u.st)), 287, 2, C_BG, statusColor(u.st));
  drawPageDots(0, 3, 312);
}

// ================================================================
// แบบ B — STATUS CARD : ดูสีและไอคอนก่อน ตัวเลขเป็นตารางเล็ก 4 ช่อง
// ================================================================
void drawStatusIcon(int cx, int cy, Status s, uint16_t col) {
  if (s == ST_NORMAL) {                       // เครื่องหมายถูก
    for (int t = 0; t < 5; t++) {
      tft.drawLine(cx - 16, cy + t, cx - 5, cy + 11 + t, col);
      tft.drawLine(cx - 5, cy + 11 + t, cx + 16, cy - 10 + t, col);
    }
  } else if (s == ST_FAST || s == ST_SLOW) {  // ลูกศรขึ้น/ลง
    int dir = (s == ST_FAST) ? -1 : 1;
    for (int i = 0; i <= 14; i++)
      tft.drawFastHLine(cx - i, cy + dir * (14 - i) - dir * 4, i * 2 + 1, col);
    tft.fillRect(cx - 5, cy - dir * 4, 11, 16, col);
  } else if (s == ST_NOFLOW) {                // กากบาท
    for (int t = 0; t < 5; t++) {
      tft.drawLine(cx - 14 + t, cy - 14, cx + 14 + t, cy + 14, col);
      tft.drawLine(cx + 14 - t, cy - 14, cx - 14 - t, cy + 14, col);
    }
  } else if (s == ST_NEAREND) {               // ถุงน้ำเกลือใกล้หมด
    tft.drawRoundRect(cx - 11, cy - 15, 22, 30, 4, col);
    tft.fillRect(cx - 9, cy + 4, 18, 9, col);
  } else {                                    // หยุดชั่วคราว
    tft.fillRect(cx - 12, cy - 13, 8, 26, col);
    tft.fillRect(cx + 4, cy - 13, 8, 26, col);
  }
}

void tile(int x, int y, const char* label, const String &value, const char* unit, uint16_t valueCol) {
  tft.fillRoundRect(x, y, 78, 40, 6, C_CARD);
  textAt(String(label), x + 7, y + 6, 1, C_DIM, C_CARD);
  textAt(value, x + 7, y + 19, 2, valueCol, C_CARD);
  if (unit[0]) textRight(String(unit), x + 72, y + 26, 1, C_DIM, C_CARD);
}

void drawConceptB(const UiState &u) {
  tft.fillScreen(C_BG);
  drawTopBar(u);

  uint16_t sc = statusColor(u.st);
  tft.fillRoundRect(6, 28, 160, 96, 10, sc);
  drawStatusIcon(86, 62, u.st, C_BG);
  textCenter(String(statusWord(u.st)), 100, 2, C_BG, sc);

  tile(6,  132, "RATE",   String(u.rate),             "mL/h", (u.st == ST_NOFLOW) ? C_RED : C_WHITE);
  tile(88, 132, "TARGET", String(u.target),           "mL/h", C_SKY);
  tile(6,  178, "LEFT",   String(u.plan - u.infused), "mL",   C_WHITE);
  tile(88, 178, "END",    String(u.endClock),         "",     C_WHITE);

  int pct = (u.plan > 0) ? (u.infused * 100 / u.plan) : 0;
  textAt("INFUSED", 8, 232, 1, C_DIM, C_BG);
  textRight(String(pct) + "%", 164, 226, 2, C_WHITE, C_BG);
  progressBar(6, 246, 160, 12, pct, (u.st == ST_NEAREND) ? C_ORANGE : C_CYAN, C_CARD);

  tft.fillRoundRect(6, 266, 160, 42, 8, C_CARD);
  textAt("TIME LEFT", 14, 274, 1, C_DIM, C_CARD);
  textRight(timeLeftStr(u, false), 162, 284, 2, C_WHITE, C_CARD);
  drawPageDots(0, 3, 314);
}

// ================================================================
// แบบ C — IV BAG : เห็นระดับน้ำในถุงและกระเปาะหยดเหมือนของจริง
// ================================================================
void drawBagGraphic(const UiState &u, int x, int y, int w, int h) {
  int pctLeft = (u.plan > 0) ? ((u.plan - u.infused) * 100 / u.plan) : 0;
  if (pctLeft < 0) pctLeft = 0;
  if (pctLeft > 100) pctLeft = 100;

  tft.fillRect(x + w / 2 - 1, y, 3, 8, C_LINE);                 // ห่วงแขวน
  tft.drawRoundRect(x, y + 8, w, h, 6, C_WHITE);

  int innerY = y + 12, innerH = h - 8;
  int fillH = innerH * pctLeft / 100;
  uint16_t liquid = (u.st == ST_NEAREND) ? C_ORANGE : C_CYAN;
  if (fillH > 3) tft.fillRoundRect(x + 4, innerY + (innerH - fillH), w - 8, fillH, 4, liquid);
  for (int i = 1; i < 4; i++)                                    // ขีดบอกระดับที่ขอบขวา
    tft.drawFastHLine(x + w - 9, innerY + innerH * i / 4, 5, C_DIM);

  int cy = y + h + 18;                                           // กระเปาะหยด
  tft.fillRect(x + w / 2 - 1, y + h + 8, 3, 10, C_LINE);
  tft.drawRoundRect(x + w / 2 - 15, cy, 30, 46, 5, C_WHITE);
  tft.fillRect(x + w / 2 - 1, cy + 4, 3, 6, C_WHITE);
  if (u.st != ST_NOFLOW && u.st != ST_PAUSED) tft.fillCircle(x + w / 2, cy + 20, 3, C_CYAN);
  tft.fillRoundRect(x + w / 2 - 12, cy + 33, 24, 10, 3, C_CYAN);
  tft.fillRect(x + w / 2 - 1, cy + 46, 3, 8, C_LINE);

  textCenterIn(String(pctLeft) + "% LEFT", x - 6, w + 12, cy + 60, 1, liquid, C_BG);
}

void drawConceptC(const UiState &u) {
  tft.fillScreen(C_BG);
  drawTopBar(u);

  drawBagGraphic(u, 12, 34, 58, 124);

  const int rx = 84;
  char rateBuf[8];
  snprintf(rateBuf, sizeof(rateBuf), "%d", u.rate);
  textAt("RATE", rx + 4, 36, 1, C_DIM, C_BG);
  if (u.st == ST_PAUSED) textAt("--", rx + 4, 48, 4, C_SKY, C_BG);
  else textAt(String(rateBuf), rx + 4, 48, 4, (u.st == ST_NOFLOW) ? C_RED : C_WHITE, C_BG);
  textAt("mL/h", rx + 4, 82, 1, C_DIM, C_BG);
  textAt("SET " + String(u.target), rx + 4, 94, 1, C_SKY, C_BG);

  tft.drawFastHLine(rx + 2, 110, 82, C_LINE);
  textAt("LEFT", rx + 4, 118, 1, C_DIM, C_BG);
  textAt(String(u.plan - u.infused) + " mL", rx + 4, 130, 2, C_WHITE, C_BG);

  tft.drawFastHLine(rx + 2, 154, 82, C_LINE);
  textAt("TIME LEFT", rx + 4, 160, 1, C_DIM, C_BG);
  textAt(timeLeftStr(u, true), rx + 4, 172, 2, C_WHITE, C_BG);
  textAt("END " + String(u.endClock), rx + 4, 196, 1, C_DIM, C_BG);

  uint16_t sc = statusColor(u.st);
  tft.fillRoundRect(rx, 214, 84, 40, 6, sc);
  textCenterIn(String(statusWord(u.st)), rx, 84, 229, 1, C_BG, sc);

  int pct = (u.plan > 0) ? (u.infused * 100 / u.plan) : 0;
  textAt("INFUSED " + String(u.infused) + "/" + String(u.plan) + " mL", 12, 272, 1, C_DIM, C_BG);
  textRight(String(pct) + "%", 160, 272, 1, C_WHITE, C_BG);
  progressBar(12, 286, 148, 12, pct, (u.st == ST_NEAREND) ? C_ORANGE : C_CYAN, C_CARD);
  drawPageDots(0, 3, 308);
}

// ---------------------------------------------------------------- main
int main(int argc, char** argv) {
  openOps(argc > 1 ? argv[1] : "ops.txt");

  UiState normal  = { 1, "14:05", 98,  100, 620, 1000, "17:20", 135, ST_NORMAL,  true };
  UiState alert   = { 1, "14:05", 0,   100, 620, 1000, "--:--", 0,   ST_NOFLOW,  true };
  UiState nearEnd = { 1, "16:42", 96,  100, 840, 1000, "17:20", 38,  ST_NEAREND, true };

  drawConceptA(normal);  mark("A1_normal");
  drawConceptA(alert);   mark("A2_alert");
  drawConceptB(normal);  mark("B1_normal");
  drawConceptB(alert);   mark("B2_alert");
  drawConceptC(normal);  mark("C1_normal");
  drawConceptC(alert);   mark("C2_alert");
  drawConceptA(nearEnd); mark("A3_nearend");
  drawConceptB(nearEnd); mark("B3_nearend");
  drawConceptC(nearEnd); mark("C3_nearend");

  closeOps();
  return 0;
}
