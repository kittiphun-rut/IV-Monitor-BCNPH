/**
 * @file      HostScreen.h
 * @brief     ทุกอย่างที่วาดลงจอ OLED 128x64 ของเครื่องส่วนกลาง
 * @version   4.7.7
 * @date      2026-09-23
 * @author    นายกิตติพันธ์ รัตนคร <kittiphun.rut@mcu.ac.th>
 *
 * @par Organization
 * วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
 * หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น จังหวัดแพร่
 * อาจารย์ที่ปรึกษา ดร.กรรณิการ์ กาศสมบูรณ์
 *
 * @par Description
 * แยกออกมาจาก `Host-OLED.ino` เพื่อให้ไฟล์หลักสั้นและหาของเจอเร็วขึ้น
 * มีหน้ารวมทุกเตียงแบบบล็อก หน้ารายเตียง หน้า LINK DIAGNOSTIC และหน้าพักจอ
 *
 * @par Revision History
 * | Version | Date | Change |
 * |---|---|---|
 * | 4.7.7 | 2026-09-23 | แยกออกมาจากไฟล์หลัก ย้ายที่อยู่ล้วน ๆ ไม่ได้แก้เนื้อในแม้แต่บรรทัดเดียว |
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
 *  @brief     ทุกอย่างที่วาดลงจอ OLED ของเครื่องส่วนกลาง
 *  @version   4.7.7-OLED
 *
 *  แยกออกมาจาก Host-OLED.ino เพื่อให้ไฟล์หลักสั้นและอ่านง่าย
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

// กระเปาะขนาดจิ๋ว กว้าง 6 px สำหรับการ์ดเตียงในหน้ารวม (h ตั้งแต่ 6 px ขึ้นไป)
void drawDripMini(int x, int y, int h, int i) {
  if (h < 6) h = 6;
  u8g2.drawFrame(x, y, 6, h);               // ตัวกระเปาะ
  u8g2.drawBox(x + 2, y + 1, 2, 1);         // หัวหยด

  switch (dripModeOf(i)) {
    case DRIP_OFFLINE:
      u8g2.drawLine(x + 1, y + 2, x + 4, y + h - 3);
      u8g2.drawLine(x + 4, y + 2, x + 1, y + h - 3);
      return;
    case DRIP_PAUSED:
      u8g2.drawVLine(x + 1, y + 2, h - 4);
      u8g2.drawVLine(x + 4, y + 2, h - 4);
      return;
    case DRIP_ALERT:
      u8g2.drawVLine(x + 2, y + 2, h - 5);
      u8g2.drawBox(x + 2, y + h - 2, 2, 1);
      return;
    default:
      break;
  }

  int poolY = y + h - 3;
  u8g2.drawBox(x + 1, poolY, 4, 2);         // ผิวน้ำในกระเปาะ

  int pct = dripAnimPhase(i);
  if (pct == -2) {                          // กระเพื่อมตอนหยดถึงผิวน้ำ
    u8g2.drawHLine(x + 1, poolY - 1, 4);
  } else if (pct >= 0) {
    int top = y + 2;
    int bot = poolY - 2;
    if (bot < top) bot = top;
    int dy  = top + ((bot - top) * pct) / 100;
    u8g2.drawBox(x + 2, dy, 2, 2);          // หยดที่กำลังตก
  }
}

// กระเปาะเต็มรูปแบบ กว้าง 15 px — ใช้ในหน้า 1-2 เตียง และหน้ารายละเอียด
void drawDripChamber(int x, int y, int h, int i) {
  u8g2.drawRFrame(x, y, 15, h, 3);
  u8g2.drawBox(x + 6, y + 2, 3, 3);         // หัวหยด

  DripMode m = dripModeOf(i);
  int poolTop = y + h - 9;

  if (m == DRIP_OFFLINE) {
    u8g2.drawLine(x + 4, y + 10, x + 10, y + h - 6);
    u8g2.drawLine(x + 10, y + 10, x + 4, y + h - 6);
    return;
  }
  if (m == DRIP_PAUSED) {
    u8g2.drawBox(x + 4, y + 11, 2, h - 20);
    u8g2.drawBox(x + 9, y + 11, 2, h - 20);
    return;
  }
  if (m == DRIP_ALERT) {
    u8g2.drawBox(x + 7, y + 10, 2, h - 22);
    u8g2.drawBox(x + 7, y + h - 10, 2, 2);
    return;
  }

  u8g2.drawBox(x + 2, poolTop, 11, 7);      // น้ำที่ก้นกระเปาะ
  int pct = dripAnimPhase(i);
  if (pct == -2) {
    u8g2.setDrawColor(0);                   // คลื่นกระเพื่อมบนผิวน้ำ
    u8g2.drawHLine(x + 3, poolTop + 1, 9);
    u8g2.setDrawColor(1);
    u8g2.drawHLine(x + 2, poolTop - 1, 11);
  } else if (pct >= 0) {
    int top = y + 7;
    int bot = poolTop - 3;
    int dy  = top + ((bot - top) * pct) / 100;
    u8g2.drawBox(x + 6, dy, 3, 3);
  }
}

// ----------------------------------------------------------------------------
// หน้ารวมทุกเตียง — เน้นอ่านจากระยะไกล
// ----------------------------------------------------------------------------
// v4.7.4 ตัดสิ่งที่มองไม่เห็นจริงบนจอ 128x64 ออก: ไม่มีกระเปาะหยด ไม่มีปริมาตรสะสม
// เหลือสามอย่างที่ตัดสินใจได้ทันที — เลขเตียง / อัตราไหล / คำสถานะ
// แล้วใช้พื้นที่ที่เหลือขยายฟอนต์จาก 4x6 เป็น 6x10 (3-6 เตียง) และ 5x8 (7-8 เตียง)
// กระเปาะหยดย้ายไปอยู่หน้ารายละเอียดของแต่ละเตียงแทน ซึ่งมีที่ให้วาดใหญ่พอจะเห็นจริง
void renderBlockStyleGrid() {
  int topY = 11;
  int bottomY = 53;
  int totalH = bottomY - topY;

  int onlineCount = 0;
  int alarmBed = 0;
  uint8_t alarmCode = ALERT_NONE;

  for (int i = 0; i < activeStationCount; i++) {
    if (isStationOnline(i)) onlineCount++;
    if (alarmBed == 0 && isCriticalAlert(stations[i].alertCode)) {
      alarmBed = i + 1;
      alarmCode = stations[i].alertCode;
    }
  }

  if (activeStationCount == 1) {
    u8g2.drawRFrame(0, topY + 1, 128, totalH - 1, 3);
    bool online = isStationOnline(0);

    if (!online || !stations[0].isRunning) {
      u8g2.setFont(u8g2_font_logisoso16_tf);
      u8g2.drawStr(14, topY + 30, online ? "PAUSED" : "OFFLINE");
    } else {
      char rBuf[16];
      snprintf(rBuf, sizeof(rBuf), "%.0f", stations[0].flowRate_ml_hr);
      u8g2.setFont(u8g2_font_logisoso32_tf);
      u8g2.drawStr(8, topY + 36, rBuf);
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(78, topY + 22, "mL/h");
      u8g2.setFont(u8g2_font_7x13_tf);
      u8g2.drawStr(78, topY + 36, shortStatusOf(0));
    }
  }
  else if (activeStationCount == 2) {
    int cardW = 62;
    for (int i = 0; i < 2; i++) {
      int x = (i == 0) ? 0 : 66;
      u8g2.drawRFrame(x, topY + 1, cardW, totalH - 1, 2);

      char idStr[10];
      snprintf(idStr, sizeof(idStr), "BED %d", i + 1);
      u8g2.setFont(u8g2_font_5x8_tf);
      u8g2.drawStr(x + 4, topY + 11, idStr);

      if (!isStationOnline(i) || !stations[i].isRunning) {
        u8g2.setFont(u8g2_font_7x13_tf);
        u8g2.drawStr(x + 4, topY + 27, isStationOnline(i) ? "PAUSE" : "OFF");
      } else {
        char rBuf[16];
        snprintf(rBuf, sizeof(rBuf), "%.0f", stations[i].flowRate_ml_hr);
        u8g2.setFont(u8g2_font_logisoso16_tf);
        u8g2.drawStr(x + 4, topY + 29, rBuf);
        u8g2.setFont(u8g2_font_5x8_tf);
        u8g2.drawStr(x + 40, topY + 29, "mL/h");
      }

      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(x + 4, topY + 39, shortStatusOf(i));
    }
  }
  else {
    int cardW = 62;
    int rows = (activeStationCount + 1) / 2;
    int rowH = (rows <= 3) ? 14 : 10;   // 3-6 เตียง = 3 แถว, 7-8 เตียง = 4 แถว
    bool bigFont = (rowH == 14);

    for (int i = 0; i < activeStationCount; i++) {
      int col = i % 2;
      int row = i / 2;
      int x = (col == 0) ? 0 : 65;
      int y = topY + row * rowH;
      int textY = y + (bigFont ? 10 : 7);

      bool attn = bedNeedsAttention(i);

      // เตียงที่ต้องไปดู = การ์ดทึบทั้งใบ มองเห็นได้จากท้ายห้อง
      if (attn) { u8g2.drawRBox(x, y, cardW, rowH - 1, 2); u8g2.setDrawColor(0); }
      else      { u8g2.drawRFrame(x, y, cardW, rowH - 1, 2); }

      u8g2.setFont(bigFont ? u8g2_font_6x10_tf : u8g2_font_5x8_tf);

      char idStr[6];
      snprintf(idStr, sizeof(idStr), "%d", i + 1);
      u8g2.drawStr(x + 3, textY, idStr);

      if (isStationOnline(i) && stations[i].isRunning) {
        char rBuf[8];
        snprintf(rBuf, sizeof(rBuf), "%3.0f", stations[i].flowRate_ml_hr);
        u8g2.drawStr(x + (bigFont ? 11 : 10), textY, rBuf);
      }

      // คำสถานะชิดขวาของการ์ดเสมอ ตาจึงกวาดหาคำผิดปกติได้เป็นแนวตั้ง
      const char *st = gridStatusOf(i);
      int adv = bigFont ? 6 : 5;
      int stX = x + cardW - 3 - (int)strlen(st) * adv;
      u8g2.drawStr(stX, textY, st);

      u8g2.setDrawColor(1);
    }
  }

  // ---- แถบล่าง: เรียงตามความเร่งด่วน ----
  u8g2.drawHLine(0, 53, 128);
  u8g2.setFont(u8g2_font_5x8_tf);
  char buf[36];
  uint8_t beyond = heardBeyondBedCount();

  if (alarmBed > 0) {
    snprintf(buf, sizeof(buf), "%sB%d %s", isSnoozed() ? "(ZZ) " : "! ", alarmBed, alertTextEn(alarmCode));
    u8g2.drawBox(0, 54, 128, 10);
    u8g2.setDrawColor(0);
    u8g2.drawStr(2, 62, buf);
    u8g2.setDrawColor(1);
  }
  else if (anyIdConflict()) {
    // สำคัญกว่าทุกอย่างที่เหลือ เพราะทำให้ข้อมูลของเตียงอื่นหายไปทั้งเตียง
    u8g2.drawBox(0, 54, 128, 10);
    u8g2.setDrawColor(0);
    u8g2.drawStr(2, 62, "! 2 NODES SAME BED ID");
    u8g2.setDrawColor(1);
  }
  else if (beyond > 0) {
    snprintf(buf, sizeof(buf), "! HEARD BED %d - ADD BEDS", beyond);
    u8g2.drawFrame(0, 54, 128, 10);
    u8g2.drawStr(2, 62, buf);
  }
  else {
    int nearBed = 0;
    for (int i = 0; i < activeStationCount; i++) {
      if (stations[i].alertCode == ALERT_NEAR_END && !stations[i].nearEndAck) { nearBed = i + 1; break; }
    }
    if (nearBed > 0) {
      const StationData &ns = stations[nearBed - 1];
      int pct = (ns.cfg.planVolumeMl > 0) ? (int)(ns.totalVolumeMl * 100.0f / ns.cfg.planVolumeMl) : 0;
      snprintf(buf, sizeof(buf), "B%d NEW BAG SOON %d%%", nearBed, pct);
      u8g2.drawFrame(0, 54, 128, 10);
      u8g2.drawStr(2, 62, buf);
    } else {
      snprintf(buf, sizeof(buf), "ONLINE %d/%d", onlineCount, activeStationCount);
      u8g2.drawStr(0, 62, buf);
    }
  }
}

void updateHostOLED() {
  if (oledDisplaySleeping || isPowerOffProgressActive) return;
  u8g2.clearBuffer();

  bool nearPending = anyNearEndPending();
  if (!oledScreensaverActive && !globalAlarmTriggered && !nearPending && (millis() - lastUserActivityTime >= 300000)) {
    oledScreensaverActive = true;
  }
  // มีเหตุวิกฤต (ยังไม่พักเสียง) หรือเตือนใกล้หมดที่ยังไม่รับทราบ -> ออกจาก Screensaver เพื่อแสดงเตือน
  if (oledScreensaverActive && ((globalAlarmTriggered && !isSnoozed()) || nearPending)) {
    oledScreensaverActive = false;
    currentOledPage = 0;
  }

  if (oledScreensaverActive) {
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(20, 10, "SMART IV CLOCK");
    u8g2.drawHLine(10, 13, 108);

    u8g2.setFont(u8g2_font_logisoso32_tf);
    String clockStr = getOledClockStr();
    u8g2.drawStr(4, 50, clockStr.c_str());

    // ที่อยู่ที่พยาบาลต้องพิมพ์ลงเบราว์เซอร์ เปลี่ยนตามโหมดเครือข่าย
    u8g2.setFont(u8g2_font_4x6_tf);
    String addr = netModeLabel() + " " + activeIP().toString();
    u8g2.drawStr((128 - (int)addr.length() * 4) / 2, 62, addr.c_str());
  }
  else if (currentOledPage == 0) {
    // หัวจอ: ชื่อหน้า + นาฬิกา (เลขจำนวนเตียงย้ายไปอยู่แถบล่างแล้ว ไม่ต้องบอกสองที่)
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(0, 7, "IV MONITOR");
    String clk = getOledClockStr();
    u8g2.drawStr(128 - (int)clk.length() * 5, 7, clk.c_str());
    u8g2.drawHLine(0, 9, 128);

    renderBlockStyleGrid();
  }
  else if (currentOledPage >= 1 && currentOledPage <= activeStationCount) {
    int idx = currentOledPage - 1;
    const StationData &s = stations[idx];
    bool isOnline = isStationOnline(idx);
    bool isPaused = isOnline && !s.isRunning;

    // ---- หัวจอ: เลขเตียงตัวใหญ่ อ่านออกทันทีว่ากำลังดูเตียงไหน ----
    char headBuf[16];
    snprintf(headBuf, sizeof(headBuf), "BED %d", currentOledPage);
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(0, 10, headBuf);
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(50, 9, shortStatusOf(idx));
    u8g2.drawHLine(0, 12, 128);

    // ---- กระเปาะหยด: ย้ายมาอยู่หน้านี้เท่านั้น มีที่พอให้วาดใหญ่จนเห็นจริง ----
    drawDripChamber(110, 15, 46, idx);

    // ---- อัตราไหล: ตัวเลขใหญ่ที่สุดบนหน้าจอ ----
    if (!isOnline || isPaused) {
      u8g2.setFont(u8g2_font_logisoso16_tf);
      u8g2.drawStr(2, 34, isOnline ? "PAUSED" : "OFFLINE");
    } else {
      char rBuf[12];
      snprintf(rBuf, sizeof(rBuf), "%.0f", s.flowRate_ml_hr);
      u8g2.setFont(u8g2_font_logisoso32_tf);
      u8g2.drawStr(2, 44, rBuf);
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(62, 26, "mL/h");
      char setBuf[16];
      snprintf(setBuf, sizeof(setBuf), "set %.0f", s.cfg.targetRateHr);
      u8g2.setFont(u8g2_font_5x8_tf);
      u8g2.drawStr(62, 40, setBuf);
    }

    // ---- แถบความคืบหน้าของถุง แทนตัวเลขปริมาตรสองชุดที่อ่านยาก ----
    int pct = (s.cfg.planVolumeMl > 0)
              ? (int)(s.totalVolumeMl * 100.0f / s.cfg.planVolumeMl) : 0;
    if (pct > 100) pct = 100;
    u8g2.drawFrame(0, 47, 104, 7);
    if (pct > 0) u8g2.drawBox(1, 48, (102 * pct) / 100, 5);

    char volBuf[24];
    if (s.cfg.planVolumeMl > 0)
      snprintf(volBuf, sizeof(volBuf), "%.0f/%.0f mL  %d%%", s.totalVolumeMl, s.cfg.planVolumeMl, pct);
    else
      snprintf(volBuf, sizeof(volBuf), "%.0f mL (no plan)", s.totalVolumeMl);
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(0, 62, volBuf);
  }
  // ---- หน้าตรวจการเชื่อมต่อ (สำหรับช่างเท่านั้น อยู่ท้ายสุดของวง) ----
  else if (currentOledPage == activeStationCount + 1) {
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(0, 7, "LINK DIAGNOSTIC");
    {
      String m = netModeLabel();
      u8g2.drawStr(128 - (int)m.length() * 5, 7, m.c_str());
    }
    u8g2.drawHLine(0, 9, 128);

    // ตารางเลขเตียง 1-8: ได้ยินไหม / MAC ท้าย 2 ไบต์ / คุณภาพลิงก์
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.drawStr(0, 16, "BED MAC   LNK  BED MAC   LNK");
    for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) {
      int col = i / 4;
      int row = i % 4;
      int x = col * 65;
      int y = 24 + row * 7;
      const StationData &s = stations[i];
      char line[24];
      if (!heardStationId(i)) {
        snprintf(line, sizeof(line), "%d   --       -", i + 1);
      } else {
        snprintf(line, sizeof(line), "%d   %02X%02X%s %3d", i + 1, s.srcMac[4], s.srcMac[5],
                 s.idConflict ? "*" : " ", s.linkPct);
      }
      if (i >= activeStationCount && heardStationId(i)) {
        u8g2.drawBox(x, y - 5, 62, 7);      // ได้ยินแต่ยังไม่ได้เปิดใช้ = กลับสีให้สะดุดตา
        u8g2.setDrawColor(0);
        u8g2.drawStr(x + 1, y, line);
        u8g2.setDrawColor(1);
      } else {
        u8g2.drawStr(x + 1, y, line);
      }
    }

    u8g2.drawHLine(0, 54, 128);
    u8g2.setFont(u8g2_font_5x8_tf);
    // คำเตือนสำคัญมาก่อนเสมอ ถ้าไม่มีคำเตือนจึงแสดงที่อยู่ที่ต้องพิมพ์
    if (anyIdConflict())            u8g2.drawStr(0, 62, "* = 2 NODES SAME ID!");
    else if (heardBeyondBedCount()) u8g2.drawStr(0, 62, "INVERTED = ADD MORE BEDS");
    else {
      String line = activeIP().toString();
      if (netMode == NET_MODE_STA) line += "  CH" + String(currentWifiChannel());
      u8g2.drawStr(0, 62, line.c_str());
    }
  }

  u8g2.sendBuffer();
}
