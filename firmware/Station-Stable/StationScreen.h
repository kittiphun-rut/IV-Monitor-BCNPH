/**
 * @file      StationScreen.h
 * @brief     ทุกอย่างที่วาดลงจอ TFT ของเครื่องประจำเตียง สายเสถียร
 * @version   7.5.5
 *
 * แยกออกมาจาก Station-Stable.ino เพื่อให้ไฟล์หลักสั้นและอ่านง่าย
 * คนที่มารับช่วงดูแลต่อจะได้หาของเจอเร็วขึ้น
 *
 * เป็นการ **ย้ายที่อยู่ล้วน ๆ** ไม่ได้แก้เนื้อในแม้แต่บรรทัดเดียว
 * พิสูจน์ด้วยการเรนเดอร์ภาพหน้าจอก่อนและหลังแยก แล้วเทียบทีละพิกเซล
 *
 * @warning ถูก #include ท้ายไฟล์หลักก่อน setup() ห้ามย้ายขึ้นไปบนสุด
 *          เพราะโค้ดในนี้ใช้ตัวแปรและฟังก์ชันช่วยที่ประกาศไว้เหนือบรรทัด include
 * @note    Arduino IDE แสดงไฟล์นี้เป็นแท็บของสเก็ตช์เดียวกัน เปิดคู่กันได้เลย
 */

#pragma once

// ---- ตัวช่วยวางข้อความ (ฟอนต์มาตรฐาน: 1 ตัวอักษร = 6xsize กว้าง, 8xsize สูง) ----
void textAt(const String &s, int x, int y, uint8_t size, uint16_t col, uint16_t bg) {
  tft.setTextSize(size);
  tft.setTextColor(col, bg);
  tft.setCursor(x, y);
  tft.print(s);
}

// ---- แถบสัญญาณ Host ----
void drawLinkBars(int x, int y, int rssi, bool online) {
  int bars = 0;
  uint16_t col = UI_GREEN;
  if (online) {
    if (rssi >= -65)      { bars = 4; col = UI_GREEN; }
    else if (rssi >= -75) { bars = 3; col = THEME_CYAN; }
    else if (rssi >= -85) { bars = 2; col = COLOR_ORANGE; }
    else                  { bars = 1; col = COLOR_RED; }
  }
  int h[4] = {4, 7, 10, 13};
  for (int i = 0; i < 4; i++) {
    int bx = x + i * 4;
    tft.fillRect(bx, y + (13 - h[i]), 3, h[i], (i < bars) ? col : UI_LINE);
  }
  if (!online) {
    tft.drawLine(x, y, x + 14, y + 13, COLOR_RED);
    tft.drawLine(x, y + 13, x + 14, y, COLOR_RED);
  }
}

// ---- แถบบนสุด: ป้ายเตียงสีตามสถานะ | นาฬิกา | สัญญาณ ----
void drawTopBar(bool force) {
  UiStatus st = uiStatusOf(uiAlertCode);
  String clockStr = getStationClockStr();
  int bars = isHostOnline ? lastHostRssi : -999;

  if (force) {
    tft.fillRect(0, 0, 172, 22, UI_BAR);
    tft.fillRect(0, 22, 172, 2, uiStatusColor(st));
    cacheStatusBar = (int)st;
    cacheClock = "";
    cacheBars = -9999;
  } else if ((int)st != cacheStatusBar) {
    cacheStatusBar = (int)st;
    tft.fillRect(0, 22, 172, 2, uiStatusColor(st));
    cacheBedChip = false;
  }

  if (!cacheBedChip || force) {
    cacheBedChip = true;
    char bed[8];
    snprintf(bed, sizeof(bed), "B%02d", currentStationId);
    tft.fillRoundRect(4, 3, 30, 16, 4, uiStatusColor(st));
    textCenterIn(String(bed), 4, 30, 7, 1, THEME_BG, uiStatusColor(st));
  }

  if (clockStr != cacheClock) {
    cacheClock = clockStr;
    textCenterIn(clockStr, 46, 80, 7, 1, COLOR_WHITE, UI_BAR);
  }

  int barKey = (bars == -999) ? -999 : (bars / 5);
  if (barKey != cacheBars) {
    cacheBars = barKey;
    tft.fillRect(148, 4, 18, 14, UI_BAR);
    drawLinkBars(150, 4, lastHostRssi, isHostOnline);
  }
}

// ---- จุดบอกหน้า ----
void drawPageDots(int activeIdx, int y) {
  int count = 4;
  int w = count * 12 - 6;
  int x = (172 - w) / 2;
  tft.fillRect(0, y - 2, 172, 8, THEME_BG);
  for (int i = 0; i < count; i++) {
    if (i == activeIdx) tft.fillRoundRect(x + i * 12, y, 8, 4, 2, THEME_SKYBLUE);
    else                tft.fillRoundRect(x + i * 12, y, 6, 4, 2, UI_LINE);
  }
}

// ---- แถบความคืบหน้า ----
void drawProgressBar(int x, int y, int w, int h, int pct, uint16_t fill) {
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  tft.fillRoundRect(x, y, w, h, h / 2, UI_CARD);
  int fw = w * pct / 100;
  if (fw >= h) tft.fillRoundRect(x, y, fw, h, h / 2, fill);
  else if (fw > 0) tft.fillRect(x, y, fw, h, fill);
}

// ระดับน้ำในถุง (วาดใหม่เมื่อเปอร์เซ็นต์เปลี่ยนเท่านั้น)
void drawBagLevel(bool force) {
  int pct = infusedPct();
  int left = (pct < 0) ? 100 : (100 - pct);
  uint16_t liquid = (uiAlertCode == ALERT_NEAR_END) ? COLOR_ORANGE : THEME_CYAN;
  if (!force && left == cacheBagPct) return;
  cacheBagPct = left;

  int innerX = BAG_X + 4, innerY = BAG_Y + 12;
  int innerW = BAG_W - 8, innerH = BAG_H - 8;
  tft.fillRect(innerX, innerY, innerW, innerH, THEME_BG);
  int fillH = innerH * left / 100;
  if (fillH > 3) tft.fillRoundRect(innerX, innerY + (innerH - fillH), innerW, fillH, 4, liquid);
  for (int i = 1; i < 4; i++)
    tft.drawFastHLine(BAG_X + BAG_W - 9, innerY + innerH * i / 4, 5, UI_DIM);

  String s = (pct < 0) ? String("-- LEFT") : (String(left) + "% LEFT");
  textCenterIn(padTo(s, 10), 2, 76, 236, 1, liquid, THEME_BG);
}

void drawChamberStatic() {
  tft.drawRoundRect(CH_X, CH_Y, CH_W, CH_H, 5, COLOR_WHITE);
  tft.fillRect(DROP_X - 1, CH_Y + 4, 3, 6, COLOR_WHITE);
}

// โหมด 0 = รอหยด, 1 = หยดกำลังตก, 2 = พักการนับ, 3 = ไม่มีการไหล
void updateDripAnimation() {
  if (currentState != STATE_NORMAL_VIEW || currentNursePage != 1 || stateNeedsRedraw) return;

  unsigned long now = millis();
  int8_t mode;
  if (!isRunning) mode = 2;
  else if (isDropFalling) mode = 1;
  else if (isLocallyOccluded()) mode = 3;
  else mode = 0;

  if (mode != animModeDrawn) {
    tft.fillRect(CH_X + 2, CH_Y + 12, CH_W - 4, CH_H - 14, THEME_BG);
    drawChamberStatic();
    if (mode == 2) {
      tft.fillRect(DROP_X - 6, CH_Y + 16, 4, 14, COLOR_YELLOW);
      tft.fillRect(DROP_X + 3, CH_Y + 16, 4, 14, COLOR_YELLOW);
    } else if (mode == 3) {
      textAt("!", DROP_X - 5, CH_Y + 15, 2, COLOR_RED, THEME_BG);
    } else {
      tft.fillCircle(DROP_X, CHAMBER_NOZZLE_Y, 2, THEME_CYAN);
    }
    if (mode != 2 && mode != 3) tft.fillRoundRect(CH_X + 3, CH_Y + 32, CH_W - 6, 10, 3, THEME_CYAN);
    animModeDrawn = mode;
    animDropPrevY = CHAMBER_NOZZLE_Y;
    lastAnimFrame = 0;
  }

  if (mode == 1) {
    if (now - lastAnimFrame < 20) return;
    lastAnimFrame = now;
    unsigned long elapsed = now - dropFallStartTime;
    if (elapsed <= DROP_FALL_DURATION) {
      animDropY = CHAMBER_NOZZLE_Y + (int)((float)elapsed / (float)DROP_FALL_DURATION * (CHAMBER_POOL_Y - CHAMBER_NOZZLE_Y));
      if (animDropY != animDropPrevY) {
        tft.fillCircle(DROP_X, animDropPrevY, 2, THEME_BG);
        tft.fillCircle(DROP_X, animDropY, 2, THEME_CYAN);
        animDropPrevY = animDropY;
      }
    } else {
      tft.fillCircle(DROP_X, animDropPrevY, 2, THEME_BG);
      tft.fillRoundRect(CH_X + 3, CH_Y + 30, CH_W - 6, 12, 3, COLOR_WHITE);   // กระเพื่อมที่ผิวน้ำ
      poolFlashUntil = now + 110;
      isDropFalling = false;
    }
  } else if (mode == 0 && poolFlashUntil > 0 && now >= poolFlashUntil) {
    tft.fillRoundRect(CH_X + 3, CH_Y + 32, CH_W - 6, 10, 3, THEME_CYAN);
    tft.fillRect(CH_X + 3, CH_Y + 30, CH_W - 6, 2, THEME_BG);
    poolFlashUntil = 0;
  }
}

void drawPage1Framework() {
  tft.fillScreen(THEME_BG);
  drawTopBar(true);

  tft.fillRect(BAG_X + BAG_W / 2 - 1, BAG_Y, 3, 8, UI_LINE);            // ห่วงแขวน
  tft.drawRoundRect(BAG_X, BAG_Y + 8, BAG_W, BAG_H, 6, COLOR_WHITE);
  tft.fillRect(DROP_X - 1, BAG_Y + BAG_H + 8, 3, 10, UI_LINE);          // สายจากถุงลงกระเปาะ
  drawChamberStatic();
  tft.fillRoundRect(CH_X + 3, CH_Y + 32, CH_W - 6, 10, 3, THEME_CYAN);  // ผิวน้ำในกระเปาะ
  tft.fillRect(DROP_X - 1, CH_Y + CH_H, 3, 8, UI_LINE);                 // สายออกจากกระเปาะ

  textAt("RATE", RIGHT_X + 4, 36, 1, UI_DIM, THEME_BG);
  textAt("mL/h", RIGHT_X + 4, 82, 1, UI_DIM, THEME_BG);
  tft.drawFastHLine(RIGHT_X + 2, 110, 82, UI_LINE);
  textAt("LEFT", RIGHT_X + 4, 118, 1, UI_DIM, THEME_BG);
  tft.drawFastHLine(RIGHT_X + 2, 154, 82, UI_LINE);
  textAt("TIME LEFT", RIGHT_X + 4, 160, 1, UI_DIM, THEME_BG);

  drawPageDots(0, 308);

  cacheBagPct = -999; cacheRate = -9999; cacheTarget = -9999; cacheLeftMl = -9999;
  cachePct = -999; cacheTimeLeft = ""; cacheEndClock = ""; cacheStatusWord = "";
  animModeDrawn = -1;
  poolFlashUntil = 0;
  stateNeedsRedraw = false;
}

void updatePage1Dynamic() {
  drawTopBar(false);
  drawBagLevel(false);

  UiStatus st = uiStatusOf(uiAlertCode);
  int rate = isRunning ? (int)(currentFlowRate_ml_hr + 0.5f) : -1;
  int target = (int)(targetRateHr + 0.5f);

  if (rate != cacheRate || target != cacheTarget) {
    cacheRate = rate; cacheTarget = target;
    tft.fillRect(RIGHT_X + 4, 48, 84, 32, THEME_BG);
    if (!isRunning) textAt("--", RIGHT_X + 4, 48, 4, THEME_SKYBLUE, THEME_BG);
    else textAt(String(rate), RIGHT_X + 4, 48, 4, (st == UI_NOFLOW) ? COLOR_RED : COLOR_WHITE, THEME_BG);
    textAt(padTo(targetRateHr > 0 ? ("SET " + String(target)) : String("SET --"), 9),
           RIGHT_X + 4, 94, 1, THEME_SKYBLUE, THEME_BG);
  }

  int left = (totalPlanMl > 0) ? remainingMlInt() : -1;
  if (left != cacheLeftMl) {
    cacheLeftMl = left;
    textAt(padTo(left >= 0 ? (String(left) + " mL") : String("-- mL"), 7), RIGHT_X + 4, 130, 2, COLOR_WHITE, THEME_BG);
  }

  String tl = timeLeftText(true);
  if (tl != cacheTimeLeft) {
    cacheTimeLeft = tl;
    textAt(padTo(tl, 7), RIGHT_X + 4, 172, 2, COLOR_WHITE, THEME_BG);
  }

  String ec = endClockText();
  if (ec != cacheEndClock) {
    cacheEndClock = ec;
    textAt(padTo("END " + ec, 11), RIGHT_X + 4, 196, 1, UI_DIM, THEME_BG);
  }

  String word = String(uiStatusWord(st));
  if (word != cacheStatusWord) {
    cacheStatusWord = word;
    uint16_t sc = uiStatusColor(st);
    tft.fillRoundRect(RIGHT_X, 214, 84, 40, 6, sc);
    textCenterIn(word, RIGHT_X, 84, 229, 1, THEME_BG, sc);
  }

  int pct = infusedPct();
  if (pct != cachePct) {
    cachePct = pct;
    String line = (totalPlanMl > 0)
      ? ("INFUSED " + String((int)totalVolumeMl) + "/" + String((int)totalPlanMl) + " mL")
      : ("INFUSED " + String((int)totalVolumeMl) + " mL");
    textAt(padTo(line, 22), 12, 272, 1, UI_DIM, THEME_BG);
    textRight(padTo(pct >= 0 ? (String(pct) + "%") : String("--"), 4), 164, 272, 1, COLOR_WHITE, THEME_BG);
    drawProgressBar(12, 286, 148, 12, (pct < 0) ? 0 : pct,
                    (uiAlertCode == ALERT_NEAR_END) ? COLOR_ORANGE : THEME_CYAN);
  }
}

// ============================================================================
// หน้า 2: PLAN — แผนการให้สารน้ำและเวลา (ตัวเลขใหญ่ 4 ค่า)
// ============================================================================
void drawStatCard(int y, const char* label, const String &value, uint16_t valueColor, bool labelOnly) {
  if (labelOnly) {
    tft.fillRoundRect(6, y, 160, 46, 6, UI_CARD);
    textAt(String(label), 14, y + 7, 1, UI_DIM, UI_CARD);
    return;
  }
  tft.fillRect(60, y + 18, 100, 26, UI_CARD);
  textRight(value, 158, y + 20, 3, valueColor, UI_CARD);
}

void drawPage2Framework() {
  tft.fillScreen(THEME_BG);
  drawTopBar(true);
  drawStatCard(30,  "TARGET   mL/h", "", 0, true);
  drawStatCard(84,  "PLAN   mL",     "", 0, true);
  drawStatCard(138, "INFUSED   mL",  "", 0, true);
  drawStatCard(192, "FINISH AT",     "", 0, true);
  drawPageDots(1, 308);
  cacheP2a = ""; cacheP2b = ""; cacheP2c = ""; cacheP2d = ""; cacheP2foot = "";
  stateNeedsRedraw = false;
}

// ============================================================================
// หน้า 3: LINK — สถานะการเชื่อมต่อและเซนเซอร์
// ============================================================================
void drawPage3Framework() {
  tft.fillScreen(THEME_BG);
  drawTopBar(true);

  tft.fillRoundRect(6, 30, 160, 60, 6, UI_CARD);
  textAt("HOST LINK", 14, 36, 1, UI_DIM, UI_CARD);

  tft.fillRoundRect(6, 98, 160, 86, 6, UI_CARD);
  textAt("SENSOR", 14, 104, 1, UI_DIM, UI_CARD);

  tft.fillRoundRect(6, 192, 160, 76, 6, UI_CARD);
  textAt("DEVICE", 14, 198, 1, UI_DIM, UI_CARD);

  drawPageDots(2, 308);
  cacheP3a = ""; cacheP3b = ""; cacheP3c = "";
  stateNeedsRedraw = false;
}

void updatePage3Dynamic() {
  drawTopBar(false);

  String linkKey = String(isHostOnline ? 1 : 0) + ":" + String(lastHostRssi / 5) + ":" + String(espnowChannel);
  if (linkKey != cacheP3a) {
    cacheP3a = linkKey;
    textAt(padTo(isHostOnline ? "ONLINE" : "NO LINK", 8), 14, 50, 2, isHostOnline ? UI_GREEN : COLOR_RED, UI_CARD);
    tft.fillRect(134, 48, 20, 16, UI_CARD);
    drawLinkBars(136, 48, lastHostRssi, isHostOnline);
    String sub = isHostOnline ? ("CH " + String(espnowChannel) + "   " + String(lastHostRssi) + " dBm")
                              : ("SEARCHING CH " + String(espnowChannel));
    textAt(padTo(sub, 22), 14, 72, 1, UI_DIM, UI_CARD);
  }

  // แสดง "ระยะเบนจากเส้นฐาน" สด ๆ : ใกล้ 0 = ช่วงว่าง, พุ่งขึ้น = หยดกำลังผ่านลำแสง
  String sensorKey = String(deltaEdge / 4) + ":" + String((int)dropPhase) + ":" +
                     String(learnAmp / 4) + ":" + String(inMotion ? 1 : 0);
  if (sensorKey != cacheP3b) {
    cacheP3b = sensorKey;
    textAt(padTo("D " + String(deltaEdge), 10), 14, 118, 2,
           (dropPhase == DP_ACTIVE) ? COLOR_YELLOW : COLOR_WHITE, UI_CARD);
    textAt(padTo("TRIG " + String(dropTriggerDelta) + "  NOISE " + String(sensorNoisePp), 22),
           14, 144, 1, UI_DIM, UI_CARD);
    String third;
    uint16_t tcol = UI_DIM;
    if (inMotion) { third = "MOVING - bag swinging"; tcol = COLOR_YELLOW; }
    else if (learnAmp > 0) third = "LEARNED " + String(learnAmp) + " / " + String(learnWidth) + "ms";
    else { third = "NOT CALIBRATED"; tcol = COLOR_ORANGE; }
    textAt(padTo(third, 22), 14, 160, 1, tcol, UI_CARD);
  }

  float bv = readStationBattery();
  String devKey = String(currentStationId) + ":" + String(safeDropFactor(dropFactor)) + ":" + String((int)(bv * 10)) + ":" + String(totalDrops);
  if (devKey != cacheP3c) {
    cacheP3c = devKey;
    char bed[12];
    snprintf(bed, sizeof(bed), "BED %02d", currentStationId);
    textAt(padTo(String(bed) + "  DF" + String(safeDropFactor(dropFactor)), 12), 14, 212, 2, COLOR_WHITE, UI_CARD);
    textAt(padTo("DROPS " + String(totalDrops), 22), 14, 236, 1, UI_DIM, UI_CARD);
    String pw = (bv > 0.5f) ? ("BATT " + String(bv, 2) + " V") : String("POWER USB");
    textAt(padTo("FW " APP_VERSION "   " + pw, 22), 14, 252, 1, UI_DIM, UI_CARD);
  }
}

void drawTrendChart() {
  const int chartH = CHART_BOT - CHART_TOP;
  const int chartW = TREND_BARS * CHART_BARW;

  float maxV = (targetRateHr > 0) ? targetRateHr : 1.0f;
  for (int i = 1; i < TREND_BARS; i++) if (trendValueAt(i) > maxV) maxV = trendValueAt(i);
  if (trendLive > maxV) maxV = trendLive;
  trendScale = niceScale(maxV * 1.1f);

  tft.fillRect(CHART_X - 3, CHART_TOP - 12, chartW + 6, chartH + 26, THEME_BG);
  for (int g = 1; g <= 3; g++) {
    int gy = CHART_BOT - (chartH * g) / 4;
    for (int x = CHART_X; x < CHART_X + chartW; x += 4) tft.drawPixel(x, gy, UI_CARD);
  }

  for (int i = 0; i < TREND_BARS; i++) {
    bool live = (i == TREND_BARS - 1);
    float v = live ? trendLive : trendValueAt(i + 1);
    bool has = live ? true : trendValidAt(i + 1);
    int x = CHART_X + i * CHART_BARW;
    int h = (int)((v / trendScale) * chartH);
    if (h > chartH) h = chartH;
    if (!has)        tft.drawFastHLine(x, CHART_BOT, CHART_BARW - 1, UI_CARD);
    else if (h <= 1) tft.drawFastHLine(x, CHART_BOT, CHART_BARW - 1, COLOR_RED);
    else             tft.fillRect(x, CHART_BOT - h, CHART_BARW - 1, h, live ? THEME_PINK : THEME_CYAN);
  }

  if (targetRateHr > 0) {                       // เส้นเป้าหมาย
    int ty = CHART_BOT - (int)((targetRateHr / trendScale) * chartH);
    if (ty < CHART_TOP) ty = CHART_TOP;
    for (int x = CHART_X; x < CHART_X + chartW; x += 6) tft.drawFastHLine(x, ty, 3, COLOR_YELLOW);
  }

  tft.drawFastHLine(CHART_X - 2, CHART_BOT + 1, chartW + 4, UI_LINE);
  textAt(padTo(String((int)trendScale) + " mL/h", 10), CHART_X, CHART_TOP - 12, 1, UI_DIM, THEME_BG);
  textAt("-30m", CHART_X, CHART_BOT + 6, 1, UI_DIM, THEME_BG);
  textRight("now", CHART_X + chartW, CHART_BOT + 6, 1, UI_DIM, THEME_BG);
  trendNeedsRedraw = false;
}

void drawTrendLiveBar() {
  if (trendLive > trendScale) { drawTrendChart(); return; }
  const int chartH = CHART_BOT - CHART_TOP;
  int x = CHART_X + (TREND_BARS - 1) * CHART_BARW;
  int h = (int)((trendLive / trendScale) * chartH);
  if (h > chartH) h = chartH;
  tft.fillRect(x, CHART_TOP, CHART_BARW - 1, chartH, THEME_BG);
  if (targetRateHr > 0) {                       // คืนเส้นเป้าหมายในช่วงที่ลบไป
    int ty = CHART_BOT - (int)((targetRateHr / trendScale) * chartH);
    if (ty >= CHART_TOP && ty <= CHART_BOT) tft.drawFastHLine(x, ty, CHART_BARW - 1, COLOR_YELLOW);
  }
  if (h <= 1) tft.drawFastHLine(x, CHART_BOT, CHART_BARW - 1, COLOR_RED);
  else        tft.fillRect(x, CHART_BOT - h, CHART_BARW - 1, h, THEME_PINK);
}

void drawPage4Framework() {
  tft.fillScreen(THEME_BG);
  drawTopBar(true);
  textAt("FLOW TREND", 12, 32, 1, UI_DIM, THEME_BG);
  textRight(padTo(targetRateHr > 0 ? ("TARGET " + String((int)(targetRateHr + 0.5f))) : String("TARGET --"), 10),
            164, 32, 1, COLOR_YELLOW, THEME_BG);
  tft.fillRoundRect(6, 216, 160, 54, 6, UI_CARD);
  textAt("NOW", 14, 222, 1, UI_DIM, UI_CARD);
  textAt("AVG 30m", 100, 222, 1, UI_DIM, UI_CARD);
  drawTrendChart();
  drawPageDots(3, 308);
  cacheP4 = "";
  stateNeedsRedraw = false;
}

// ============================================================================
// จอเตือนเหตุวิกฤต (เต็มจอสีแดง)
// ============================================================================
void drawEmergencyScreen(uint8_t code) {
  tft.fillScreen(COLOR_RED);
  tft.fillRoundRect(12, 12, 148, 32, 6, COLOR_WHITE);
  char bed[12];
  snprintf(bed, sizeof(bed), "BED %02d", currentStationId);
  drawCenteredString(String(bed), 20, 2, COLOR_RED, COLOR_WHITE);

  const char* headline = "ALERT";
  const char* sub      = "CHECK IV SYSTEM";
  if (code == ALERT_OCCLUSION)      { headline = "NO FLOW";  sub = "CHECK IV LINE"; }
  else if (code == ALERT_TOO_FAST)  { headline = "TOO FAST"; sub = "ADJUST ROLLER CLAMP"; }
  else if (code == ALERT_TOO_SLOW)  { headline = "TOO SLOW"; sub = "ADJUST ROLLER CLAMP"; }
  else if (code == ALERT_COMPLETE)  { headline = "COMPLETE"; sub = "INFUSION FINISHED"; }

  drawCenteredString(String(headline), 62, 3, COLOR_WHITE, COLOR_RED);
  drawCenteredString(String(sub), 96, 1, COLOR_YELLOW, COLOR_RED);

  tft.fillRoundRect(12, 118, 148, 100, 8, THEME_BG);
  textAt("RATE", 22, 126, 1, UI_DIM, THEME_BG);
  textAt("mL/h", 22, 186, 1, UI_DIM, THEME_BG);
  textRight(targetRateHr > 0 ? ("SET " + String((int)(targetRateHr + 0.5f))) : String("SET --"),
            150, 186, 1, THEME_SKYBLUE, THEME_BG);

  tft.fillRoundRect(14, 230, 144, 62, 8, COLOR_WHITE);
  drawCenteredString("CLICK = SNOOZE 2min", 244, 1, COLOR_RED, COLOR_WHITE);
  drawCenteredString("DOUBLE = PAUSE", 268, 1, COLOR_RED, COLOR_WHITE);

  lastEmergencyCode = code;
  cacheRate = -9999;
  stateNeedsRedraw = false;
}

void updateEmergencyDynamic() {
  int rate = (int)(currentFlowRate_ml_hr + 0.5f);
  if (rate == cacheRate) return;
  cacheRate = rate;
  tft.fillRect(20, 140, 132, 40, THEME_BG);
  textAt(String(rate), 22, 142, 5, COLOR_WHITE, THEME_BG);
}

// ============================================================================
// จอเตือนใกล้หมดถุง (สีส้ม ไม่ใช่เหตุวิกฤต)
// ============================================================================
void drawNearEndNoticeFramework() {
  tft.fillScreen(COLOR_ORANGE);
  tft.fillRoundRect(12, 12, 148, 32, 6, COLOR_WHITE);
  char bed[12];
  snprintf(bed, sizeof(bed), "BED %02d", currentStationId);
  drawCenteredString(String(bed), 20, 2, COLOR_ORANGE, COLOR_WHITE);

  drawCenteredString("NEXT BAG", 60, 3, THEME_BG, COLOR_ORANGE);
  drawCenteredString("PREPARE NEW IV BAG", 94, 1, THEME_BG, COLOR_ORANGE);

  tft.fillRoundRect(12, 114, 148, 104, 8, THEME_BG);
  tft.fillRoundRect(14, 236, 144, 58, 8, COLOR_WHITE);
  drawCenteredString("CLICK = OK", 248, 2, COLOR_ORANGE, COLOR_WHITE);
  drawCenteredString("(stop reminder)", 274, 1, THEME_BG, COLOR_WHITE);

  cacheNear = "";
  stateNeedsRedraw = false;
}

// ============================================================================
// Screensaver — เห็นเฉพาะสิ่งที่ต้องดูจากระยะไกล
// ============================================================================
void drawScreensaverFramework() {
  tft.fillScreen(THEME_BG);
  UiStatus st = uiStatusOf(uiAlertCode);
  char bed[12];
  snprintf(bed, sizeof(bed), "BED %02d", currentStationId);
  tft.fillRoundRect(46, 20, 80, 28, 6, uiStatusColor(st));
  textCenterIn(String(bed), 46, 80, 26, 2, THEME_BG, uiStatusColor(st));
  drawCenteredString("mL/h", 232, 1, UI_DIM, THEME_BG);
  drawCenteredString("CLICK TO WAKE", 296, 1, UI_DIM, THEME_BG);
  cacheClock = ""; cacheRate = -9999; cacheStatusWord = "";
  stateNeedsRedraw = false;
}

void updateScreensaverDynamic() {
  String clockStr = getStationClockStr();
  if (clockStr != cacheClock) {
    cacheClock = clockStr;
    drawCenteredString(clockStr, 76, 5, COLOR_WHITE, THEME_BG);
  }

  UiStatus st = uiStatusOf(uiAlertCode);
  String word = String(uiStatusWord(st));
  if (word != cacheStatusWord) {
    cacheStatusWord = word;
    tft.fillRect(6, 136, 160, 20, THEME_BG);
    drawCenteredString(word, 138, 2, uiStatusColor(st), THEME_BG);
  }

  int rate = isRunning ? (int)(currentFlowRate_ml_hr + 0.5f) : -1;
  if (rate != cacheRate) {
    cacheRate = rate;
    tft.fillRect(6, 180, 160, 44, THEME_BG);
    drawCenteredString(rate >= 0 ? String(rate) : String("--"), 182, 5,
                       (st == UI_NOFLOW) ? COLOR_RED : COLOR_WHITE, THEME_BG);
  }
}

// ============================================================================
// หน้าผู้พัฒนา / หน้าตั้งหมายเลขเตียง
// ============================================================================
void drawDeveloperCreditScreen() {
  tft.fillScreen(THEME_BG);
  tft.fillRoundRect(6, 16, 160, 34, 6, THEME_PINK);
  drawCenteredString("SMART IV ALERT", 26, 2, THEME_BG, THEME_PINK);

  drawCenteredString("Bed Station Node", 66, 1, UI_DIM, THEME_BG);
  drawCenteredString("FW v" APP_VERSION, 82, 1, THEME_SKYBLUE, THEME_BG);

  tft.fillRoundRect(6, 106, 160, 96, 8, UI_CARD);
  drawCenteredString("DEVELOPER", 114, 1, UI_DIM, UI_CARD);
  drawCenteredString("Kittiphan", 132, 2, COLOR_YELLOW, UI_CARD);
  drawCenteredString("Rattanakorn", 154, 2, COLOR_YELLOW, UI_CARD);
  drawCenteredString("MCU Phrae Campus", 180, 1, UI_DIM, UI_CARD);

  tft.fillRoundRect(6, 212, 160, 60, 8, UI_CARD);
  drawCenteredString("BCN Phrae Innovation", 224, 1, THEME_CYAN, UI_CARD);
  drawCenteredString("Suk Mean Hospital Trial", 244, 1, UI_DIM, UI_CARD);

  drawCenteredString("CLICK TO RETURN", 296, 1, UI_DIM, THEME_BG);
  stateNeedsRedraw = false;
}

// ----------------------------------------------------------------------------
// ทิศการแสดงผลของจอ
// ----------------------------------------------------------------------------
// setRotation(0) กับ (2) เป็นแนวตั้ง 172x320 เหมือนกัน ต่างกันแค่บิต MX/MY ใน
// MADCTL พิกัดที่ใช้วาดทั้งไฟล์จึงไม่ต้องแก้ เพียงสั่งวาดหน้าเดิมซ้ำอีกครั้ง
void applyScreenRotation(uint8_t rot) {
  screenRotation = (rot == TFT_ROT_FLIP180) ? TFT_ROT_FLIP180 : TFT_ROT_NORMAL;
  tft.setRotation(screenRotation);
  tft.setTextWrap(false);
  tft.fillScreen(THEME_BG);
  stateNeedsRedraw = true;      // ให้ทุกหน้าวาดกรอบใหม่ทั้งหน้า (แคชถูกล้างในนั้น)
}

void drawStationIdConfigFramework() {
  tft.fillScreen(THEME_BG);
  tft.fillRoundRect(6, 16, 160, 34, 6, THEME_SKYBLUE);
  drawCenteredString("SET BED ID", 26, 2, THEME_BG, THEME_SKYBLUE);
  tft.fillRoundRect(6, 62, 160, 150, 8, UI_CARD);

  // แถวตั้งค่าทิศจอ — ช่วยให้ติดตั้งกล่องได้ทั้งแบบสาย USB ลงล่างและขึ้นบน
  tft.fillRoundRect(6, 220, 160, 42, 8, UI_CARD);
  drawCenteredString("SCREEN", 226, 1, UI_DIM, UI_CARD);
  drawCenteredString(padTo(screenRotationLabel(), 10), 240, 2, THEME_CYAN, UI_CARD);

  drawCenteredString("CLICK = BED 1-8", 272, 1, UI_DIM, THEME_BG);
  drawCenteredString("DBL CLICK = FLIP", 284, 1, UI_DIM, THEME_BG);
  cacheConfig = "";
  stateNeedsRedraw = false;
}

void updateStationIdConfigDynamic() {
  long msLeft = (long)configAutoSaveTimeout - (long)millis();
  int leftSec = (msLeft > 0) ? (int)(msLeft / 1000) + 1 : 0;
  String key = String(tempConfigStationId) + ":" + String(leftSec);
  if (key == cacheConfig) return;
  cacheConfig = key;

  tft.fillRect(40, 84, 92, 108, UI_CARD);
  drawCenteredString(String(tempConfigStationId), 90, 12, THEME_SKYBLUE, UI_CARD);
  drawCenteredString(padTo("SAVING IN " + String(leftSec) + "s", 16), 302, 1, COLOR_YELLOW, THEME_BG);
}
