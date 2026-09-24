/**
 * @file      StationScreen.h
 * @brief     ทุกอย่างที่วาดลงจอ OLED 0.42 นิ้ว (72x40) ของเครื่องประจำเตียงรุ่นบอร์ดเล็ก
 * @version   1.2.0-C3
 * @date      2026-09-23
 * @author    นายกิตติพันธ์ รัตนคร <kittiphun.rut@mcu.ac.th>
 *
 * @par Organization
 * วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
 * หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น จังหวัดแพร่
 * อาจารย์ที่ปรึกษา ดร.กรรณิการ์ กาศสมบูรณ์
 *
 * @par Description
 * จอกว้าง 72 สูง 40 พิกเซล วางตัวอักษร 4x6 ได้ 18 ตัวต่อบรรทัด ราว 5 บรรทัด
 * จึงออกแบบด้วยหลักสามข้อ หนึ่งหน้าจอหนึ่งคำตอบ หัวจอไม่เคยหาย
 * และเหตุที่ต้องรีบไปดูคนไข้กินทั้งจอ
 *
 * @par Revision History
 * | Version | Date | Change |
 * |---|---|---|
 * | 1.2.0-C3 | 2026-09-24 | เพิ่มหน้าจอของตัวช่วยคาลิเบรตสี่ขั้นตอน |
 * | 1.0.0-C3 | 2026-09-23 | ออกแบบหน้าจอ 72x40 ใหม่ทั้งหมดสำหรับบอร์ด ESP32-C3 |
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
 *  @file      StationScreen.h
 *  @brief     ทุกอย่างที่วาดลงจอ OLED 0.42" (72x40) ของเครื่องประจำเตียงรุ่นบอร์ดเล็ก
 *  @version   1.0.0-C3
 *
 *  แยกออกมาจาก Station-C3-OLED.ino เพื่อให้ไฟล์หลักสั้นและอ่านง่าย
 *
 *  ข้อจำกัดของจอนี้ และวิธีออกแบบให้พยาบาลอ่านได้จริง
 *
 *  จอกว้าง 72 สูง 40 พิกเซล เล็กมาก เทียบแล้วได้ตัวอักษร 4x6 เพียง 18 ตัวต่อบรรทัด
 *  และวางได้ราว 5 บรรทัดเท่านั้น จึงออกแบบด้วยหลักสามข้อ
 *
 *    1) หนึ่งหน้าจอ หนึ่งคำตอบ — แต่ละหน้าตอบคำถามเดียว ตัวเลขที่ตอบคำถามนั้น
 *       ใช้ฟอนต์ใหญ่ที่สุดที่จอรับได้ (logisoso16 สูง 16 พิกเซล = 40% ของความสูงจอ)
 *       พยาบาลเดินผ่านปลายเตียงแล้วเหลือบมองครั้งเดียวต้องอ่านออก
 *
 *    2) สิ่งที่ต้องรู้ตลอดเวลาอยู่บนหัวจอเสมอ — เลขเตียง สถานะการเชื่อมต่อ และนาฬิกา
 *       ไม่ว่าจะเปิดหน้าไหนอยู่ หัวจอไม่เคยหาย
 *
 *    3) เหตุที่ต้องรีบไปดูคนไข้ กินทั้งจอ — หน้าเตือนกลับสีขาวดำทั้งหน้าและกะพริบ
 *       เพราะการเตือนเล็ก ๆ ที่มุมจอบนจอขนาดนี้ไม่มีใครเห็น
 *
 *  ตัวอักษรบนจอเป็นภาษาอังกฤษล้วน เพราะฟอนต์ในตัวของ U8g2 ไม่มีอักษรไทย
 *  ใช้คำสั้นที่พยาบาลคุ้นอยู่แล้ว และเลี่ยงศัพท์ที่ต้องแปลสองต่อ
 *
 *  ผังพิกเซลที่ใช้จริง (นับจากมุมบนซ้าย)
 *
 *    แถว  0- 7   หัวจอ  เลขเตียง | สัญญาณ | นาฬิกา        (ฟอนต์ 4x6 เส้นฐาน y=6)
 *    แถว  8      เส้นคั่น
 *    แถว  9-31   เนื้อหาหลักของแต่ละหน้า
 *    แถว 32-39   แถบล่าง แถบความคืบหน้า หรือข้อมูลเสริม
 *
 *  @warning ถูก #include ท้ายไฟล์หลักก่อน setup() ห้ามย้ายขึ้นไปบนสุด
 *           เพราะโค้ดในนี้ใช้ตัวแปรและฟังก์ชันช่วยที่ประกาศไว้เหนือบรรทัด include
 *  @note    Arduino IDE แสดงไฟล์นี้เป็นแท็บของสเก็ตช์เดียวกัน เปิดคู่กันได้เลย
 */

#pragma once

#define SCR_W   72
#define SCR_H   40

// ความกว้างต่อตัวอักษรของฟอนต์ความกว้างคงที่ ใช้คำนวณการจัดกึ่งกลาง
#define ADV_4X6   4
#define ADV_5X8   5
#define ADV_7X13  7
#define ADV_LOGI  11

// ---------------------------------------------------------------------------
// ตัวช่วยวาด
// ---------------------------------------------------------------------------
void drawCenteredStr(int y, int adv, const char *s) {
  int w = (int)strlen(s) * adv;
  int x = (SCR_W - w) / 2;
  if (x < 0) x = 0;
  u8g2.drawStr(x, y, s);
}

void drawRightStr(int rightEdge, int y, int adv, const char *s) {
  int x = rightEdge - (int)strlen(s) * adv;
  if (x < 0) x = 0;
  u8g2.drawStr(x, y, s);
}

String clockStr() {
  time_t now;
  time(&now);
  struct tm *t = localtime(&now);
  char b[8];
  if (isClockSynced && t) strftime(b, sizeof(b), "%H:%M", t);
  else                    snprintf(b, sizeof(b), "--:--");
  return String(b);
}

// สัดส่วนที่ให้ไปแล้วเทียบกับแผน (0..100) — ใช้ทั้งแถบความคืบหน้าและตัวเลข %
int progressPct() {
  if (totalPlanMl <= 0.0f) return -1;          // Host ยังไม่ได้ส่งแผนมา
  int p = (int)((totalVolumeMl / totalPlanMl) * 100.0f + 0.5f);
  if (p < 0)   p = 0;
  if (p > 100) p = 100;
  return p;
}

// ---------------------------------------------------------------------------
// หัวจอ — มีทุกหน้า ไม่เคยหาย
//   ซ้าย  เลขเตียง
//   กลาง  สถานะการเชื่อมต่อกับเครื่องส่วนกลาง
//   ขวา   นาฬิกา (หรือคำว่า PAUSE กะพริบเมื่อหยุดชั่วคราว)
// ---------------------------------------------------------------------------
void drawHeader() {
  u8g2.setFont(u8g2_font_4x6_tf);

  char bed[6];
  snprintf(bed, sizeof(bed), "B%d", stationId);
  u8g2.drawStr(0, 6, bed);

  // สัญญาณ: ต่อติด = แท่งทึบสามขีดไล่ระดับ / หลุด = คำว่า NO LINK กะพริบ
  if (isHostOnline()) {
    u8g2.drawBox(12, 4, 2, 2);
    u8g2.drawBox(15, 2, 2, 4);
    u8g2.drawBox(18, 0, 2, 6);
  } else if ((millis() / 500) % 2 == 0) {
    u8g2.drawStr(12, 6, "NO LINK");
  }

  if (!isRunning) {
    if ((millis() / 500) % 2 == 0) drawRightStr(SCR_W, 6, ADV_4X6, "PAUSE");
  } else {
    drawRightStr(SCR_W, 6, ADV_4X6, clockStr().c_str());
  }

  u8g2.drawHLine(0, 8, SCR_W);
}

// ---------------------------------------------------------------------------
// กระเปาะหยด — แสดง "การนับ" ให้เห็นด้วยตา ไม่ใช่แค่ตัวเลข
//
// จังหวะหยดล็อกกับเวลาจริงของหยดล่าสุด ไม่ใช่อนิเมชันลอย ๆ
// หยดที่เห็นบนจอจึงตกพร้อมกับหยดจริงในกระเปาะ พยาบาลใช้เทียบด้วยตาได้
// ---------------------------------------------------------------------------
void drawDripChamber(int x, int y, int w, int h) {
  u8g2.drawFrame(x, y, w, h);
  u8g2.drawBox(x + w / 2 - 1, y + 1, 2, 3);        // หัวหยดด้านบน

  int liquidTop = y + h - 6;
  u8g2.drawBox(x + 1, liquidTop, w - 2, 5);        // ระดับน้ำก้นกระเปาะ

  if (!isRunning || !hasFirstDropOccurred) return;

  unsigned long meanIv = rateWindowMeanIntervalMs();
  if (meanIv < 200) return;                        // ยังไม่รู้จังหวะ ไม่ต้องเดา

  unsigned long since = msSinceLastDrop();
  if (since > meanIv * 3) return;                  // หยดหยุดไปแล้ว อย่าวาดให้เข้าใจผิด

  unsigned long phase = since % meanIv;
  int travel = liquidTop - (y + 5);
  int dy = (int)((float)phase / (float)meanIv * travel);
  u8g2.drawBox(x + w / 2 - 1, y + 5 + dy, 2, 2);   // หยดที่กำลังตก
}

// ---------------------------------------------------------------------------
// แถบความคืบหน้าของถุง
// ---------------------------------------------------------------------------
void drawProgressBar() {
  int pct = progressPct();
  if (pct < 0) {
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.drawStr(0, 39, "no plan from host");
    return;
  }
  u8g2.drawFrame(0, 33, 52, 6);
  int fill = (52 - 2) * pct / 100;
  if (fill > 0) u8g2.drawBox(1, 34, fill, 4);

  char b[8];
  snprintf(b, sizeof(b), "%d%%", pct);
  u8g2.setFont(u8g2_font_4x6_tf);
  drawRightStr(SCR_W, 39, ADV_4X6, b);
}

// ---------------------------------------------------------------------------
// หน้า 1 — อัตราการไหล  ตอบคำถาม "ตอนนี้ไหลเท่าไร ตรงตามแผนไหม"
// ---------------------------------------------------------------------------
void drawPageRate() {
  char b[12];

  if (!isRunning) {
    u8g2.setFont(u8g2_font_7x13_tf);
    drawCenteredStr(26, ADV_7X13, "PAUSED");
    u8g2.setFont(u8g2_font_4x6_tf);
    drawCenteredStr(38, ADV_4X6, "double tap = run");
    return;
  }

  float r = currentFlowRate_ml_hr;
  if (r >= 100.0f)     snprintf(b, sizeof(b), "%.0f", r);
  else if (r >= 10.0f) snprintf(b, sizeof(b), "%.0f", r);
  else                 snprintf(b, sizeof(b), "%.1f", r);
  u8g2.setFont(u8g2_font_logisoso16_tf);
  u8g2.drawStr(0, 29, b);

  // คอลัมน์ขวาเริ่มที่ x=38 เพราะตัวเลขสามหลักของ logisoso16 กว้างราว 35 px
  // ถ้าวางที่ 35 ตัวอักษรตัวแรกจะถูกเลขหลักสุดท้ายทับ (เคยพลาดมาแล้ว)
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(38, 19, "mL/h");
  // สถานะเซนเซอร์มาก่อนค่าเป้าหมายเสมอ ถ้าตัวตรวจจับยังเรียนรู้ไม่เสร็จ
  // ตัวเลขบนจอยังเชื่อไม่ได้ พยาบาลต้องรู้ข้อนี้ก่อนรู้ว่าเป้าหมายเท่าไร
  if (det().lock != iv::Lock::Ready) {
    u8g2.drawStr(38, 28, sensorStateWord());
  } else if (targetRateHr > 0.0f) {
    snprintf(b, sizeof(b), ">%.0f", targetRateHr);   // > คือค่าที่แพทย์สั่ง
    u8g2.drawStr(38, 28, b);
  } else {
    u8g2.drawStr(38, 28, "no plan");
  }

  drawDripChamber(58, 11, 13, 21);
  drawProgressBar();
}

// ---------------------------------------------------------------------------
// หน้า 2 — หยดต่อนาที  ตอบคำถาม "นับหยดเทียบกับที่ตั้งไว้ตรงไหม"
// เป็นหน่วยที่พยาบาลใช้นับด้วยตาจริงที่ข้างเตียง จึงแยกเป็นหน้าของตัวเอง
// ---------------------------------------------------------------------------
void drawPageGtt() {
  char b[12];

  u8g2.setFont(u8g2_font_logisoso16_tf);
  snprintf(b, sizeof(b), "%.0f", isRunning ? currentGttMin : 0.0f);
  u8g2.drawStr(0, 29, b);

  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(38, 19, "gtt");
  u8g2.drawStr(38, 28, "/min");

  drawDripChamber(58, 11, 13, 21);

  if (targetRateHr > 0.0f) {
    float tGtt = targetRateHr * safeDropFactor(dropFactor) / 60.0f;
    snprintf(b, sizeof(b), "target %.1f", tGtt);
  } else {
    snprintf(b, sizeof(b), "factor %d gtt/mL", safeDropFactor(dropFactor));
  }
  u8g2.drawStr(0, 39, b);
}

// ---------------------------------------------------------------------------
// หน้า 3 — ยอดสะสม  ตอบคำถาม "ให้ไปแล้วเท่าไร นับได้กี่หยด"
// ---------------------------------------------------------------------------
void drawPageVolume() {
  char b[16];

  u8g2.setFont(u8g2_font_helvB10_tf);
  snprintf(b, sizeof(b), "%.0f", totalVolumeMl);
  u8g2.drawStr(0, 21, b);

  u8g2.setFont(u8g2_font_4x6_tf);
  if (totalPlanMl > 0.0f) snprintf(b, sizeof(b), "/%.0f mL", totalPlanMl);
  else                    snprintf(b, sizeof(b), "mL");
  u8g2.drawStr(34, 21, b);

  u8g2.drawHLine(0, 24, SCR_W);

  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(0, 33, "DROPS");
  snprintf(b, sizeof(b), "%lu", (unsigned long)totalDrops);
  drawRightStr(SCR_W, 33, ADV_5X8, b);

  u8g2.setFont(u8g2_font_4x6_tf);
  snprintf(b, sizeof(b), "sensor %s", sensorStateWord());
  u8g2.drawStr(0, 39, b);
}

// ---------------------------------------------------------------------------
// หน้า 4 — สถานะเครื่อง  สำหรับช่างและตอนติดตั้ง ไม่ใช่หน้าที่พยาบาลใช้ประจำ
// ---------------------------------------------------------------------------
void drawPageStatus() {
  char b[24];
  u8g2.setFont(u8g2_font_4x6_tf);

  snprintf(b, sizeof(b), "BED %d", stationId);
  u8g2.drawStr(0, 15, b);
  snprintf(b, sizeof(b), "ch %d", espnowChannel);
  drawRightStr(SCR_W, 15, ADV_4X6, b);

  u8g2.drawStr(0, 22, isHostOnline() ? "host ONLINE" : "host SEARCHING");

  snprintf(b, sizeof(b), "sensor %s", sensorStateWord());
  u8g2.drawStr(0, 29, b);

  if (batteryVolts >= 0.5f) snprintf(b, sizeof(b), "bat %.2fV %d%%", batteryVolts, batteryPct());
  else                      snprintf(b, sizeof(b), "USB power");
  u8g2.drawStr(0, 36, b);
}

// ---------------------------------------------------------------------------
// หน้าเตือน — กินทั้งจอและกะพริบ
// จอขนาดนี้ถ้าเตือนเล็ก ๆ ที่มุมจอจะไม่มีใครเห็น จึงกลับสีทั้งหน้าสลับไปมา
// ---------------------------------------------------------------------------
void drawAlertScreen() {
  const char *title  = "ALERT";
  const char *action = "";

  switch (uiAlertCode) {
    case ALERT_OCCLUSION: title = "NO FLOW";  action = "check line / clamp"; break;
    case ALERT_TOO_FAST:  title = "TOO FAST"; action = "slow the roller";    break;
    case ALERT_TOO_SLOW:  title = "TOO SLOW"; action = "open the roller";    break;
    case ALERT_NEAR_END:  title = "NEAR END"; action = "prepare a new bag";  break;
    case ALERT_COMPLETE:  title = "FINISHED"; action = "IV plan completed";  break;
  }

  bool invert = ((millis() / 600) % 2) == 0;
  if (invert) {
    u8g2.drawBox(0, 0, SCR_W, SCR_H);
    u8g2.setDrawColor(0);
  }

  u8g2.setFont(u8g2_font_7x13_tf);
  drawCenteredStr(17, ADV_7X13, title);

  u8g2.setFont(u8g2_font_4x6_tf);
  drawCenteredStr(27, ADV_4X6, action);

  char b[20];
  if (uiAlertCode == ALERT_NEAR_END) snprintf(b, sizeof(b), "BED %d  tap = OK", stationId);
  else                               snprintf(b, sizeof(b), "BED %d", stationId);
  drawCenteredStr(37, ADV_4X6, b);

  if (invert) u8g2.setDrawColor(1);
}

// ---------------------------------------------------------------------------
// หน้าตั้งเลขเตียง — เข้าด้วยการกดค้าง 4 วินาที
// ของเดิมต้องแก้ #define แล้วคอมไพล์ใหม่ทีละเตียง ใช้จริงในหอผู้ป่วยไม่ไหว
// ---------------------------------------------------------------------------
void drawSetIdScreen() {
  u8g2.setFont(u8g2_font_5x8_tf);
  drawCenteredStr(8, ADV_5X8, "SET BED ID");
  u8g2.drawHLine(0, 10, SCR_W);

  char b[4];
  snprintf(b, sizeof(b), "%d", stationId);
  u8g2.setFont(u8g2_font_logisoso16_tf);
  drawCenteredStr(30, ADV_LOGI, b);

  u8g2.setFont(u8g2_font_4x6_tf);
  drawCenteredStr(39, ADV_4X6, "tap +1  hold save");
}

// ---------------------------------------------------------------------------
// หน้าพักจอ — ลดการเบิร์นของจอ OLED และยังบอกสิ่งที่จำเป็นที่สุดสองอย่าง
// ---------------------------------------------------------------------------
void drawSaverScreen() {
  u8g2.setFont(u8g2_font_logisoso16_tf);
  drawCenteredStr(24, ADV_LOGI, clockStr().c_str());

  char b[20];
  u8g2.setFont(u8g2_font_4x6_tf);
  if (isRunning) snprintf(b, sizeof(b), "BED %d  %.0f mL/h", stationId, currentFlowRate_ml_hr);
  else           snprintf(b, sizeof(b), "BED %d  PAUSED", stationId);
  drawCenteredStr(37, ADV_4X6, b);
}

// ---------------------------------------------------------------------------
// ตัวช่วยคาลิเบรตสี่ขั้นตอน ([1.2.0-C3] เพิ่มทั้งหมด)
//
// จอ 72x40 เล็กเกินกว่าจะแสดงกราฟสัญญาณแบบสายหลัก จึงใช้หลัก "หนึ่งหน้าจอ
// หนึ่งคำตอบ" ให้สุดทาง แต่ละขั้นบอกสามอย่างเท่านั้น
//   บนสุด  อยู่ขั้นไหนจากสี่ขั้น (ตัวเลขและช่องสี่ช่อง มองแวบเดียวรู้ว่าเหลืออีกกี่ขั้น)
//   กลาง   ค่าที่กำลังตัดสินขั้นนี้ ตัวใหญ่สุดที่จอรับได้
//   ล่างสุด สิ่งที่ต้องลงมือทำกับสาย ไม่ใช่สิ่งที่เครื่องกำลังทำ
//
// ผังพิกเซล
//   แถว  0- 6  "CAL n/4" ซ้าย · ช่องบอกความคืบหน้าสี่ช่องขวา
//   แถว  8     เส้นคั่น
//   แถว  9-17  ชื่อขั้น (5x8)
//   แถว 19-31  ค่าที่ตัดสินขั้นนี้ (7x13)
//   แถว 33-39  สิ่งที่ต้องทำ หรือสาเหตุที่ไม่ผ่าน (4x6)
// ---------------------------------------------------------------------------
void drawCalibSteps(int y) {
  // สี่ช่องบอกความคืบหน้า ทึบ = ผ่านแล้ว · กะพริบ = ขั้นที่กำลังทำ · โปร่ง = ยังไม่ถึง
  for (int i = 0; i < 4; i++) {
    int x = 49 + i * 6;          // ช่องสุดท้ายจบที่ x=71 พอดีขอบจอ 72 พิกเซล
    if (i < (int)calStep)        u8g2.drawBox(x, y, 5, 5);
    else if (i == (int)calStep)  { if ((millis() / 350) % 2 == 0) u8g2.drawBox(x, y, 5, 5);
                                   else                          u8g2.drawFrame(x, y, 5, 5); }
    else                          u8g2.drawFrame(x, y, 5, 5);
  }
}

// ค่าที่ตัดสินขั้นนี้ เขียนลง buf ให้สั้นที่สุดเท่าที่ยังอ่านรู้เรื่อง
void calBigValue(char *buf, size_t n) {
  const iv::DetectorStatus &d = det();
  switch (calStep) {
    case CAL_BASELINE:
      // ระหว่างที่เข้าเกณฑ์แล้ว นับถอยหลังให้เห็น จะได้รู้ว่าต้องนิ่งอีกนานแค่ไหน
      if (calOkSince != 0) {
        unsigned long left = CAL_QUIET_MS - (millis() - calOkSince);
        snprintf(buf, n, "%lus", (unsigned long)(left / 1000 + 1));
      } else if (sensorRaw > CAL_RAW_MAX) {
        snprintf(buf, n, "RAW HI");      // ชนเพดาน ADC — แสงแรงเกินหรือเซนเซอร์หลุด
      } else if (sensorRaw < CAL_RAW_MIN) {
        snprintf(buf, n, "RAW LO");      // ต่ำผิดปกติ — สายหลุดหรือเลนส์ถูกบัง
      } else {
        snprintf(buf, n, "n %d", d.noiseSigma);   // เหลือกรณีเดียวคือสัญญาณรบกวนสูง
      }
      break;
    case CAL_DIRECTION:
      // ยังสรุปทิศไม่ได้ ให้บอกจำนวนหยดที่เห็นแล้ว พยาบาลจะได้รู้ว่ามันเห็นหยดอยู่
      // ไม่ใช่ค้างเฉย ๆ · สรุปได้แล้วจึงขึ้นทิศ ด้วยถ้อยคำเดียวกับสายหลัก
      if (calDirSign == 0) snprintf(buf, n, "%lu dr", (unsigned long)calDropsThisStep());
      else                 snprintf(buf, n, "%s", calDirSign < 0 ? "DROP LO" : "DROP HI");
      break;
    case CAL_LEARN:
      snprintf(buf, n, "%lu/%d", (unsigned long)calDropsThisStep(), CAL_LEARN_DROPS);
      break;
    case CAL_VERIFY:
      snprintf(buf, n, "%lu/%d", (unsigned long)calDropsThisStep(), CAL_VERIFY_DROPS);
      break;
    default:
      snprintf(buf, n, "PASS");
      break;
  }
}

void drawCalibScreen() {
  char b[16];

  u8g2.setFont(u8g2_font_4x6_tf);
  if (calStep == CAL_DONE) snprintf(b, sizeof(b), "CAL OK");
  else                     snprintf(b, sizeof(b), "CAL %d/4", (int)calStep + 1);
  u8g2.drawStr(0, 6, b);
  drawCalibSteps(1);
  u8g2.drawHLine(0, 8, SCR_W);

  u8g2.setFont(u8g2_font_5x8_tf);
  if (calStep == CAL_DONE) {
    // หน้าสรุปเอาค่า SNR ขึ้นมาแทนชื่อขั้น เพราะเป็นตัวเลขเดียวที่บอกว่า
    // ตำแหน่งเซนเซอร์ดีพอหรือยัง ช่างจะได้เทียบกันได้ระหว่างเตียง
    if (calResultSnrX10 >= 999) snprintf(b, sizeof(b), "SNR 99+");
    else snprintf(b, sizeof(b), "SNR %d.%d", calResultSnrX10 / 10, calResultSnrX10 % 10);
    drawCenteredStr(17, ADV_5X8, b);
  } else {
    drawCenteredStr(17, ADV_5X8, calStepName(calStep));
  }

  // ขีดใต้ชื่อขั้นเมื่อเข้าเกณฑ์แล้ว บอกว่ากำลังจะผ่าน ไม่ใช่ยังลุ้นอยู่
  if (calOkSince != 0 && calStep != CAL_DONE) u8g2.drawHLine(6, 19, SCR_W - 12);

  // ขั้นที่หมดเวลา: กลับสีทั้งแถบกลาง เพราะบนจอขนาดนี้ตัวหนังสือเล็ก ๆ ไม่มีใครเห็น
  if (calFailed && (millis() / 400) % 2 == 0) {
    u8g2.drawBox(0, 19, SCR_W, 13);
    u8g2.setDrawColor(0);
  }
  calBigValue(b, sizeof(b));
  u8g2.setFont(u8g2_font_7x13_tf);
  drawCenteredStr(31, ADV_7X13, b);
  u8g2.setDrawColor(1);

  u8g2.setFont(u8g2_font_4x6_tf);
  if (calStep == CAL_DONE) {
    drawCenteredStr(39, ADV_4X6, "press to save");
  } else if (calFailed) {
    drawCenteredStr(39, ADV_4X6, calFailHint);
  } else {
    drawCenteredStr(39, ADV_4X6, calStepHint(calStep));
  }
}

// ---------------------------------------------------------------------------
// หน้าต้อนรับตอนเปิดเครื่อง
// ---------------------------------------------------------------------------
void drawSplash() {
  u8g2.clearBuffer();
  u8g2.drawRFrame(0, 0, SCR_W, SCR_H, 3);

  u8g2.setFont(u8g2_font_5x8_tf);
  drawCenteredStr(12, ADV_5X8, "SMART IV");

  u8g2.setFont(u8g2_font_4x6_tf);
  drawCenteredStr(21, ADV_4X6, "BED STATION");
  drawCenteredStr(29, ADV_4X6, "v" APP_VERSION);

  char b[12];
  snprintf(b, sizeof(b), "BED %d", stationId);
  drawCenteredStr(37, ADV_4X6, b);

  u8g2.sendBuffer();
}

// ---------------------------------------------------------------------------
// ตัวเลือกหน้าจอหลัก — เรียกจาก loop()
// ---------------------------------------------------------------------------
void drawScreen() {
  u8g2.clearBuffer();

  switch (uiMode) {
    case MODE_ALERT:  drawAlertScreen();  u8g2.sendBuffer(); return;
    case MODE_SET_ID: drawSetIdScreen();  u8g2.sendBuffer(); return;
    case MODE_SAVER:  drawSaverScreen();  u8g2.sendBuffer(); return;
    case MODE_CALIB:  drawCalibScreen();  u8g2.sendBuffer(); return;   // [1.2.0-C3] เพิ่ม
    default: break;
  }

  drawHeader();
  switch (currentPage) {
    case PAGE_RATE:   drawPageRate();   break;
    case PAGE_GTT:    drawPageGtt();    break;
    case PAGE_VOLUME: drawPageVolume(); break;
    case PAGE_STATUS: drawPageStatus(); break;
    default: break;
  }
  u8g2.sendBuffer();
}
