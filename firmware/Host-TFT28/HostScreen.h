/**
 * @file      HostScreen.h
 * @brief     ทุกอย่างที่วาดลงจอสี TFT 2.8 นิ้ว ของเครื่องส่วนกลาง
 * @version   4.8.1
 * @date      2026-09-23
 * @author    นายกิตติพันธ์ รัตนคร <kittiphun.rut@mcu.ac.th>
 *
 * @par Organization
 * วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
 * หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น จังหวัดแพร่
 * อาจารย์ที่ปรึกษา ดร.กรรณิการ์ กาศสมบูรณ์
 *
 * @par Description
 * แยกออกมาจาก `Host-TFT28.ino` เพื่อให้ไฟล์หลักสั้นและหาของเจอเร็วขึ้น
 * หน้าจอแนวทาง FOCUS ครึ่งซ้ายคือเตียงที่ต้องดูตอนนี้ ครึ่งขวาคือเตียงอื่น
 *
 * @par Revision History
 * | Version | Date | Change |
 * |---|---|---|
 * | 4.8.1 | 2026-09-23 | แยกออกมาจากไฟล์หลัก ย้ายที่อยู่ล้วน ๆ ไม่ได้แก้เนื้อในแม้แต่บรรทัดเดียว |
 *
 * @warning  ถูก `#include` ท้ายไฟล์หลักก่อน `setup()` ห้ามย้ายขึ้นไปบนสุด
 *           เพราะโค้ดในนี้ใช้ตัวแปรและฟังก์ชันช่วยที่ประกาศไว้เหนือบรรทัด include
 *           ถ้าโค้ดเหนือบรรทัดนั้นเรียกฟังก์ชันในไฟล์นี้ ต้องประกาศล่วงหน้าในไฟล์หลักด้วย
 *           ตรวจด้วย `bash tools/split-check/run.sh`
 *
 * @warning  ข้อความที่วาดลงจอต้องเป็น ASCII ล้วน ฟอนต์ในตัวของไลบรารีไม่มีอักษรไทย
 *           อักษรไทยหนึ่งตัวกินสามไบต์ จะกลายเป็นสัญลักษณ์มั่วสามตัวและกินที่กว้างกว่าที่นับไว้
 *
 * @note     Arduino IDE แสดงไฟล์นี้เป็นแท็บของสเก็ตช์เดียวกัน เปิดคู่กับไฟล์หลักได้เลย
 *
 * @par บันทึกการเปลี่ยนแปลงโดยละเอียด
 * เก็บข้อความเดิมไว้ทั้งหมด เพราะเหตุผลเชิงเทคนิคในนั้นหาจากที่อื่นไม่ได้
 *  @file      HostScreen.h
 *  @brief     ทุกอย่างที่วาดลงจอ TFT 2.8" ของเครื่องส่วนกลาง
 *  @version   4.8.1
 *
 *  แยกออกมาจาก Host-TFT28.ino เพื่อให้ไฟล์หลักสั้นและอ่านง่าย
 *  คนที่มารับช่วงดูแลต่อจะได้หาของเจอเร็วขึ้น
 *
 *  เป็นการ **ย้ายที่อยู่ล้วน ๆ** ไม่ได้แก้เนื้อในแม้แต่บรรทัดเดียว
 *  พิสูจน์ด้วยการเรนเดอร์ภาพหน้าจอก่อนและหลังแยก แล้วเทียบทีละพิกเซล
 *
 *  @warning ถูก #include ท้ายไฟล์หลักก่อน setup() ห้ามย้ายขึ้นไปบนสุด
 *           เพราะโค้ดในนี้ใช้ตัวแปรและฟังก์ชันช่วยที่ประกาศไว้เหนือบรรทัด include
 *  @note    Arduino IDE แสดงไฟล์นี้เป็นแท็บของสเก็ตช์เดียวกัน เปิดคู่กันได้เลย
 */

#pragma once

// ---- ตัวช่วยวางข้อความ (ฟอนต์มาตรฐาน: 1 ตัวอักษร = 6xsize กว้าง, 8xsize สูง) ----
void textAt(const String &s, int x, int y, uint8_t size, uint16_t col, uint16_t bg) {
  tft.setTextSize(size);
  tft.setTextColor(col, bg);
  tft.setCursor(x, y);
  tft.print(s);
}

void drawProgressBar(int x, int y, int w, int h, int pct, uint16_t fill) {
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  tft.fillRoundRect(x, y, w, h, h / 2, C_BAR);
  int fw = w * pct / 100;
  if (fw >= h) tft.fillRoundRect(x, y, fw, h, h / 2, fill);
  else if (fw > 0) tft.fillRect(x, y, fw, h, fill);
}

void drawWifiIcon(int x, int y, int clients) {
  int h[4] = {3, 6, 9, 12};
  for (int i = 0; i < 4; i++) tft.fillRect(x + i * 4, y + (12 - h[i]), 3, h[i], (clients > i) ? C_GREEN : C_LINE);
}

void drawBatteryIcon(int x, int y, int pct) {
  tft.drawRoundRect(x, y, 22, 11, 2, C_DIM);
  tft.fillRect(x + 22, y + 3, 2, 5, C_DIM);
  tft.fillRect(x + 2, y + 2, 18, 7, C_BG);
  uint16_t c = (pct <= 20) ? C_RED : (pct <= 50 ? C_YELLOW : C_GREEN);
  int w = (18 * pct) / 100;
  if (w > 0) tft.fillRect(x + 2, y + 2, w, 7, c);
}

// ---- แถบบนสุด: ชื่อเครื่อง | นาฬิกา | จำนวนเครื่องที่ต่อ Wi-Fi + แบตเตอรี่ ----
void drawTopBarFramework() {
  tft.fillRect(0, 0, SCR_W, TOPBAR_H, C_TOPBAR);
  tft.fillRect(0, TOPBAR_H, SCR_W, 2, C_LINE);
  textAt("SMART IV HOST", 8, 8, 1, C_SKY, C_TOPBAR);
}

// ---- แถบสรุปเหตุการณ์ด้านล่าง ----
void updateBottomBar() {
  int alarm = -1, near = -1;
  for (int i = 0; i < activeStationCount; i++) {
    if (alarm < 0 && bedIsAlarm(bedUiStatus(i))) alarm = i;
    if (near < 0 && stations[i].alertCode == ALERT_NEAR_END && !stations[i].nearEndAck) near = i;
  }
  int online = 0;
  for (int i = 0; i < activeStationCount; i++) if (isStationOnline(i)) online++;

  char buf[48];
  uint16_t bg, fg;
  if (alarm >= 0) {
    snprintf(buf, sizeof(buf), "! BED %02d  %s  -  CHECK NOW", alarm + 1, alertTextEn(stations[alarm].alertCode));
    bg = C_RED; fg = C_WHITE;
  } else if (near >= 0) {
    snprintf(buf, sizeof(buf), "BED %02d  PREPARE NEXT BAG", near + 1);
    bg = C_ORANGE; fg = C_BG;
  } else {
    snprintf(buf, sizeof(buf), "ALL NORMAL   ONLINE %d/%d", online, activeStationCount);
    bg = C_CARD; fg = C_GREEN;
  }
  String line = String(buf);
  if (line == cacheBottom) return;
  cacheBottom = line;
  tft.fillRect(0, BOTBAR_Y, SCR_W, BOTBAR_H, bg);
  textCenterIn(line, 0, SCR_W, BOTBAR_Y + 5, 1, fg, bg);
}

// ---- รายการเตียงด้านขวา ----
void drawSideRow(int idx, int y, int rowH) {
  BedUiStatus st = bedUiStatus(idx);
  const StationData &s = stations[idx];
  bool dim = (st == BU_OFFLINE);
  uint16_t card = dim ? C_OFFCARD : C_CARD;
  uint16_t sc = bedColor(st);

  tft.fillRoundRect(SIDE_X, y, SIDE_W, rowH, 4, card);
  tft.fillRect(SIDE_X, y, 4, rowH, sc);

  char bed[8];
  snprintf(bed, sizeof(bed), "B%02d", idx + 1);
  String rateStr;
  if (st == BU_OFFLINE)     rateStr = "OFF";
  else if (st == BU_PAUSED) rateStr = "||";
  else                      rateStr = String((int)(s.flowRate_ml_hr + 0.5f));

  if (rowH >= 30) {
    textAt(String(bed), SIDE_X + 8, y + 5, 2, dim ? C_DIM : C_WHITE, card);
    textRight(rateStr, SIDE_X + SIDE_W - 6, y + 5, 2, bedIsAlarm(st) ? sc : (dim ? C_DIM : C_WHITE), card);
    if (st != BU_OFFLINE) drawProgressBar(SIDE_X + 8, y + rowH - 9, SIDE_W - 16, 5, bedPct(idx),
                                          (st == BU_NEAREND) ? C_ORANGE : C_CYAN);
  } else {
    textAt(String(bed), SIDE_X + 8, y + (rowH - 8) / 2, 1, dim ? C_DIM : C_WHITE, card);
    textRight(rateStr, SIDE_X + SIDE_W - 6, y + (rowH - 16) / 2, 2, bedIsAlarm(st) ? sc : (dim ? C_DIM : C_WHITE), card);
  }
}

void updateSideList(int focus, bool force) {
  int n = activeStationCount - 1;
  if (n <= 0) return;
  int gap = 4;
  int rowH = (PANEL_H - gap * (n - 1)) / n;
  if (rowH > 40) rowH = 40;
  if (rowH < 20) rowH = 20;
  if (force || rowH != uiRowsDrawn) {
    tft.fillRect(SIDE_X, PANEL_Y, SIDE_W, PANEL_H, C_BG);
    uiRowsDrawn = rowH;
    for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) cacheSide[i] = "";
  }

  int slot = 0;
  for (int i = 0; i < activeStationCount; i++) {
    if (i == focus) continue;
    BedUiStatus st = bedUiStatus(i);
    String key = String((int)st) + ":" + String((int)(stations[i].flowRate_ml_hr + 0.5f)) + ":" + String(bedPct(i));
    if (key != cacheSide[i]) {
      cacheSide[i] = key;
      drawSideRow(i, PANEL_Y + slot * (rowH + gap), rowH);
    }
    slot++;
  }
}

// ---- แผงเตียงหลักด้านซ้าย ----
void drawFocusPanel(int focus, bool force) {
  BedUiStatus st = bedUiStatus(focus);
  const StationData &s = stations[focus];
  uint16_t sc = bedColor(st);

  char head[24];
  snprintf(head, sizeof(head), "BED %02d", focus + 1);
  String headKey = String(head) + "|" + String((int)st) + "|" + String(s.cfg.patientName);
  if (force || headKey != cacheFocusHead) {
    cacheFocusHead = headKey;
    tft.fillRoundRect(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, 6, C_CARD);
    tft.fillRoundRect(PANEL_X, PANEL_Y, PANEL_W, 26, 6, sc);
    tft.fillRect(PANEL_X, PANEL_Y + 18, PANEL_W, 8, sc);
    textAt(String(head), PANEL_X + 7, PANEL_Y + 6, 2, C_BG, sc);
    String name = String(s.cfg.patientName);
    if (name.length() > 14) name = name.substring(0, 14);
    textRight(name, PANEL_X + PANEL_W - 7, PANEL_Y + 8, 1, C_BG, sc);
    cacheStatusWord = ""; cacheRate = ""; cacheSetRate = ""; cacheSummary = ""; cachePct = -999;
  }

  String word = String(bedWord(st));
  if (word != cacheStatusWord) {
    cacheStatusWord = word;
    textAt(padTo(word, 9), PANEL_X + 9, 66, 2, sc, C_CARD);
  }

  String rateStr = (st == BU_OFFLINE) ? String("--") :
                   (st == BU_PAUSED)  ? String("--") : String((int)(s.flowRate_ml_hr + 0.5f));
  if (rateStr != cacheRate) {
    cacheRate = rateStr;
    tft.fillRect(PANEL_X + 9, 92, PANEL_W - 18, 48, C_CARD);
    textAt(rateStr, PANEL_X + 9, 92, 6, bedIsAlarm(st) ? sc : C_WHITE, C_CARD);
  }

  String setStr = (s.cfg.targetRateHr > 0) ? ("SET " + String((int)(s.cfg.targetRateHr + 0.5f))) : String("SET --");
  if (setStr != cacheSetRate) {
    cacheSetRate = setStr;
    textAt("mL/h", PANEL_X + 9, 148, 1, C_DIM, C_CARD);
    textRight(padTo(setStr, 9), PANEL_X + PANEL_W - 9, 148, 1, C_SKY, C_CARD);
  }

  int pct = bedPct(focus);
  if (pct != cachePct) {
    cachePct = pct;
    drawProgressBar(PANEL_X + 9, 168, PANEL_W - 18, 12, (pct < 0) ? 0 : pct,
                    (st == BU_NEAREND) ? C_ORANGE : C_CYAN);
  }

  int left = (s.cfg.planVolumeMl > 0) ? (int)(s.cfg.planVolumeMl - s.totalVolumeMl) : -1;
  if (left < 0 && s.cfg.planVolumeMl > 0) left = 0;
  int mins = bedMinutesLeft(focus);
  char tl[16];
  if (mins >= 0) snprintf(tl, sizeof(tl), "%dh %02dm", mins / 60, mins % 60);
  else           snprintf(tl, sizeof(tl), "--h --m");
  String sum = String(pct) + "|" + String(left) + "|" + String(tl);
  if (sum != cacheSummary) {
    cacheSummary = sum;
    tft.fillRect(PANEL_X + 7, 190, PANEL_W - 14, 10, C_CARD);
    textAt(pct >= 0 ? (String(pct) + "%") : String("--"), PANEL_X + 9, 190, 1, C_DIM, C_CARD);
    textCenterIn(left >= 0 ? (String(left) + " mL") : String("-- mL"), PANEL_X, PANEL_W, 190, 1, C_WHITE, C_CARD);
    textRight(String(tl), PANEL_X + PANEL_W - 9, 190, 1, bedIsAlarm(st) ? sc : C_WHITE, C_CARD);
  }
}

// ---- หน้าหลัก ----
void drawMainFramework() {
  tft.fillScreen(C_BG);
  drawTopBarFramework();
  resetUiCaches();
}

// ---- จอเตือนเต็มจอ ----
void drawAlarmFramework() {
  int bed = autoFocusBed();
  if (bed < 0) bed = 0;
  const StationData &s = stations[bed];
  uint8_t code = s.alertCode;

  tft.fillScreen(C_RED);
  tft.fillRoundRect(10, 8, 300, 40, 8, C_WHITE);
  char head[24];
  snprintf(head, sizeof(head), "BED %02d", bed + 1);
  textAt(String(head), 22, 18, 3, C_RED, C_WHITE);
  String name = String(s.cfg.patientName);
  if (name.length() > 16) name = name.substring(0, 16);
  textRight(name, 298, 24, 1, C_RED, C_WHITE);

  const char* word = alertTextEn(code);
  const char* sub  = "CHECK IV SYSTEM";
  if (code == ALERT_OCCLUSION)     sub = "LINE BLOCKED - CHECK TUBING";
  else if (code == ALERT_TOO_FAST) sub = "FLOW ABOVE TARGET - ADJUST CLAMP";
  else if (code == ALERT_TOO_SLOW) sub = "FLOW BELOW TARGET - ADJUST CLAMP";
  else if (code == ALERT_COMPLETE) sub = "INFUSION FINISHED";
  textCenterIn(String(word), 0, SCR_W, 62, 4, C_WHITE, C_RED);
  textCenterIn(String(sub), 0, SCR_W, 102, 1, C_YELLOW, C_RED);

  tft.fillRoundRect(20, 122, 280, 66, 8, C_BG);
  textAt("RATE", 32, 130, 1, C_DIM, C_BG);
  textAt("mL/h", 32, 172, 1, C_DIM, C_BG);
  textRight(s.cfg.targetRateHr > 0 ? ("SET " + String((int)(s.cfg.targetRateHr + 0.5f)) + " mL/h") : String("SET --"),
            288, 172, 1, C_SKY, C_BG);
  int pct = bedPct(bed);
  textRight(pct >= 0 ? ("INFUSED " + String(pct) + "%") : String("INFUSED --"), 288, 130, 1, C_DIM, C_BG);

  tft.fillRoundRect(10, 198, 300, 32, 8, C_WHITE);
  textCenterIn("PRESS POWER = SNOOZE 2 MIN", 0, SCR_W, 208, 1, C_RED, C_WHITE);
  cacheAlarmRate = "";
}

void updateAlarmScreen() {
  int bed = autoFocusBed();
  if (bed < 0) bed = 0;
  String rate = String((int)(stations[bed].flowRate_ml_hr + 0.5f));
  if (rate == cacheAlarmRate) return;
  cacheAlarmRate = rate;
  tft.fillRect(30, 140, 180, 32, C_BG);
  textAt(rate, 32, 140, 4, C_WHITE, C_BG);
}

// ---- Screensaver: นาฬิกาใหญ่ ----
void drawSaverFramework() {
  tft.fillScreen(C_BG);
  textCenterIn("SMART IV HOST", 0, SCR_W, 24, 1, C_DIM, C_BG);
  textCenterIn("PRESS ANY BUTTON TO WAKE", 0, SCR_W, 210, 1, C_DIM, C_BG);
  cacheClock = ""; cacheBottom = "";
}

// ---- หน้าจอเฉพาะกิจ: ปิดเครื่อง / กำลังกดค้าง / หน้าจอต้อนรับ ----
void drawPowerOffProgress(int pct) {
  if (uiMode != UI_MODE_POWER) {
    uiMode = UI_MODE_POWER;
    tft.fillScreen(C_BG);
    textCenterIn("POWER OFF ?", 0, SCR_W, 70, 3, C_WHITE, C_BG);
    textCenterIn("KEEP HOLDING TO TURN OFF", 0, SCR_W, 110, 1, C_DIM, C_BG);
    tft.drawRoundRect(58, 138, 204, 24, 6, C_WHITE);
  }
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  int w = (200 * pct) / 100;
  tft.fillRect(60, 140, w, 20, C_RED);
  tft.fillRect(60 + w, 140, 200 - w, 20, C_BG);
}

void drawGoodbyeScreen() {
  uiMode = UI_MODE_POWER;
  tft.fillScreen(C_BG);
  textCenterIn("GOODBYE", 0, SCR_W, 80, 4, C_SKY, C_BG);
  textCenterIn("RELEASE BUTTON TO TURN OFF", 0, SCR_W, 140, 1, C_DIM, C_BG);
  textCenterIn("HOLD 2s TO POWER ON AGAIN", 0, SCR_W, 160, 1, C_DIM, C_BG);
}

void setDisplaySleep(bool sleep) {
  displaySleeping = sleep;
#if TFT_BLK >= 0
  digitalWrite(TFT_BLK, sleep ? LOW : HIGH);
#endif
  if (sleep) {
    tft.fillScreen(C_BG);
  } else {
    uiMode = -1;          // บังคับวาดกรอบใหม่ทั้งหมดเมื่อเปิดจอ
    resetUiCaches();
  }
}

void drawSplashScreen() {
  tft.fillScreen(C_BG);
  tft.fillRoundRect(30, 46, 260, 90, 10, C_TOPBAR);
  textCenterIn("SMART IV", 0, SCR_W, 62, 4, C_WHITE, C_TOPBAR);
  textCenterIn("CENTRAL HOST", 0, SCR_W, 104, 2, C_SKY, C_TOPBAR);
  textCenterIn("Host v" APP_VERSION "   Protocol v3", 0, SCR_W, 156, 1, C_DIM, C_BG);
  char buf[40];
  snprintf(buf, sizeof(buf), "BEDS %d   AP %s", activeStationCount, default_ap_ssid);
  textCenterIn(String(buf), 0, SCR_W, 176, 1, C_DIM, C_BG);
  textCenterIn("BCN Phrae Innovation", 0, SCR_W, 200, 1, C_GREEN, C_BG);
}
