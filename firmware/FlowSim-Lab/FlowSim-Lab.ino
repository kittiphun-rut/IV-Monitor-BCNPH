/**
 * @file      FlowSim-Lab.ino
 * @brief     เครื่องมือทดลองและสาธิตการตรวจจับหยด ทำงานเดี่ยว ไม่เชื่อมต่อเครื่องส่วนกลาง
 * @version   S1.0.0
 * @date      2026-09-12
 * @author    นายกิตติพันธ์ รัตนคร <kittiphun.rut@mcu.ac.th>
 *
 * @par Organization
 * วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
 * หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น จังหวัดแพร่
 * อาจารย์ที่ปรึกษา ดร.กรรณิการ์ กาศสมบูรณ์
 *
 * @par Hardware
 * ESP32-S3 Super Mini + จอ TFT 1.47" (ST7789 SPI 172x320) + Passive Buzzer
 *
 * @par Description
 * ดัดแปลงจากเครื่องประจำเตียง v7.3.0 โดยตัดการเชื่อมต่อทุกชนิดออก
 * ไม่มี Wi-Fi ไม่มี ESP-NOW ไม่มีเครื่องส่วนกลาง เหลือเฉพาะการตรวจจับหยด
 * และการคาลิเบรตเซนเซอร์ เพิ่มกราฟรูปคลื่นและกราฟแท่งไว้ดูความไวของเซนเซอร์
 * มีโหมดจำลองสร้างสัญญาณหยดสังเคราะห์ ใช้สาธิตได้แม้ไม่ต่อเซนเซอร์
 *
 * @par Revision History
 * | Version | Date | Change |
 * |---|---|---|
 * | S1.0.0 | 2026-09-12 | สร้างเครื่องมือทดลองจากเครื่องประจำเตียง v7.3.0 โดยตัดการเชื่อมต่อออกทั้งหมด |
 *
 * @warning  ยังไม่ได้ทดสอบบนฮาร์ดแวร์จริง ตรวจด้วยเครื่องมือจำลองบนเครื่อง PC เท่านั้น
 *
 * @note     ไฟล์นี้ไม่มีโครงสร้างแพ็กเก็ต Protocol v3 เพราะไม่ได้คุยกับใคร
 *           จึงไม่อยู่ในขอบเขตของ `tools/protocol-test/run.sh`
 *
 * @par บันทึกการเปลี่ยนแปลงโดยละเอียด
 * เก็บข้อความเดิมไว้ทั้งหมด เพราะเหตุผลเชิงเทคนิคในนั้นหาจากที่อื่นไม่ได้
 *  Arduino-ESP32 Core: 3.x
 *
 *  จุดประสงค์ของเวอร์ชันนี้
 *   - ใช้เป็น "แบบจำลองกราฟแสดงสถานะการไหล" บนหน้าจอ TFT สำหรับพัฒนา/สาธิต/สอนงาน
 *   - ตัดการเชื่อมต่อทุกชนิดออก: ไม่มี Wi-Fi, ไม่มี ESP-NOW, ไม่มี Host, ไม่มีนาฬิกา NTP
 *   - เหลือเฉพาะ 2 งานหลักคือ (1) การตรวจจับหยด และ (2) การคาลิเบรตเซนเซอร์
 *   - เพิ่มกราฟรูปคลื่น (waveform) และกราฟแท่ง (bar graph) เพื่อดูแนวโน้มการไหล
 *     และดู "ความไว (sensitivity)" ของเซนเซอร์แบบเห็นภาพจริง
 *   - มีโหมดจำลอง (SIM) สร้างสัญญาณหยดสังเคราะห์ ใช้สาธิตหน้าจอได้แม้ไม่ต่อเซนเซอร์
 *
 *  3 หน้าจอหลัก (คลิก 1 ครั้งเพื่อเปลี่ยนหน้า)
 *   [1] WAVE   — รูปคลื่นค่า ADC สด + เส้นเกณฑ์ตัดสิน + แถบพัลส์การนับหยด
 *   [2] TREND  — กราฟแท่งแนวโน้มอัตราไหล (gtt/min) ย้อนหลัง 60 วินาที
 *   [3] SENSOR — ค่าคาลิเบรต, เกจตำแหน่งสัญญาณ, สัญญาณรบกวน และคะแนนความไว
 *
 *  ปุ่มกดเดียว (BTN_PIN) ทำได้ทุกคำสั่ง
 *   1 คลิก  = เปลี่ยนหน้า            2 คลิก  = RUN / HOLD (หยุดพักการนับ)
 *   3 คลิก  = ล้างสถิติทั้งหมด        4 คลิก  = เปิด/ปิดโหมดจำลองและเลือกอัตรา
 *   กด 3 วิ = เข้า Calibration Wizard  กด 5 วิ = หน้าวิธีใช้/ผู้พัฒนา
 */

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <Preferences.h>
#include <math.h>

#define APP_VERSION   "S1.0.0"

// ----------------------------------------------------------------------------
// ขาต่อใช้งาน (เหมือนเครื่องจริงทุกประการ เพื่อให้ย้ายโค้ดกลับไปใช้ได้ทันที)
// ----------------------------------------------------------------------------
#define TFT_CS        9
#define TFT_DC        10
#define TFT_RST       11
#define TFT_MOSI      12
#define TFT_SCLK      13
#define TFT_BLK       8

#define SENSOR_AO_PIN 4
#define BUZZER_PIN    3
#define BTN_PIN       2
#define RGB_LED_PIN   48

// ----------------------------------------------------------------------------
// ค่าคงที่ของการวัด
// ----------------------------------------------------------------------------
#define DROP_FACTOR        20        // gtt/mL ของชุดให้สารน้ำที่ใช้ทดลอง (10/15/20/60)
#define MIN_DROP_INTERVAL  55UL      // กันนับซ้ำ (ms) — เท่ากับเครื่องจริง
#define SOUND_ON_DROP      true      // เสียงติ๊กสั้นทุกหยด ช่วยฟังจังหวะขณะปรับเซนเซอร์

#define THEME_BG      0x0000
#define THEME_SKYBLUE 0x5DFF
#define THEME_CYAN    0x07FF
#define THEME_PINK    0xFD79
#define THEME_NAVY    0x09CD
#define THEME_DARKBOX 0x18C5
#define THEME_MAROON  0x6000

#define COLOR_WHITE   0xFFFF
#define COLOR_GREEN   0x07E0
#define COLOR_RED     0xF800
#define COLOR_ORANGE  0xFD20
#define COLOR_YELLOW  0xFFE0

#ifndef ST77XX_LIGHTGREY
  #define ST77XX_LIGHTGREY 0xC618
#endif
#ifndef ST77XX_DARKGREY
  #define ST77XX_DARKGREY  0x39E7
#endif

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  #define LED_WRITE(pin, r, g, b) rgbLedWrite(pin, r, g, b)
#else
  #define LED_WRITE(pin, r, g, b) neopixelWrite(pin, r, g, b)
#endif

SPIClass SPI_TFT(FSPI);
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI_TFT, TFT_CS, TFT_DC, TFT_RST);
Preferences labPrefs;

// ----------------------------------------------------------------------------
// ผังหน้าจอ (จอแนวตั้ง 172 x 320)
// ----------------------------------------------------------------------------
#define SCR_W          172
#define SCR_H          320
#define TOPBAR_H       22
#define FOOTER_Y       302

// หน้า 1: กราฟรูปคลื่น
#define WAVE_COLS      152                 // 1 คอลัมน์ = 1 พิกเซล = 1 ช่วงเวลา
#define WAVE_BUCKET_MS 20UL                // 152 x 20ms = ~3 วินาทีต่อหน้าจอ
#define WAVE_X         12
#define WAVE_Y         92
#define WAVE_H         100
#define PULSE_X        12
#define PULSE_Y        216
#define PULSE_H        14

// หน้า 2: กราฟแท่งแนวโน้ม
#define TREND_BARS     30
#define TREND_BIN_MS   2000UL              // 30 แท่ง x 2 วินาที = ย้อนหลัง 60 วินาที
#define CHART_X        12
#define CHART_BAR_W    5                   // แท่งกว้าง 4 + เว้น 1
#define CHART_TOP      110
#define CHART_BOT      216

enum AppState { STATE_MAIN, STATE_ABOUT };
enum PageId   { PAGE_WAVE = 1, PAGE_TREND = 2, PAGE_SENSOR = 3 };

AppState currentState = STATE_MAIN;
int  currentPage      = PAGE_WAVE;
bool needRedraw       = true;

// ---- สถานะการนับหยด -------------------------------------------------------
bool     isRunning            = true;
bool     dropState            = false;
uint32_t totalDrops           = 0;
float    currentFlowRate_ml_hr = 0.0f;
float    currentGttMin        = 0.0f;
float    totalVolumeMl        = 0.0f;
bool     hasFirstDropOccurred = false;
bool     rateAnchorValid      = false;
unsigned long lastDropTimestamp = 0;
unsigned long lastDebounceTime  = 0;

// ---- ค่าคาลิเบรตเซนเซอร์ (ใช้ namespace เดียวกับเครื่องจริง st_cal) --------
int  valLiquid     = 1200;   // ADC ขณะมีสารน้ำ/หยดผ่านลำแสง
int  valAir        = 2800;   // ADC ขณะกระเปาะว่าง (ค่าฐาน)
int  dropThreshold = 2000;
int  hysteresis    = 200;
bool dropLowersAdc = true;   // true = หยดทำให้ค่า ADC ลดลง

// ---- ค่าที่อ่านได้สด ------------------------------------------------------
int rawAdc = 0;

// ---- สถิติสัญญาณรบกวน (ดูความไวของเซนเซอร์) -------------------------------
int  noiseP2P        = 0;    // สัญญาณรบกวนของเส้นฐาน (0 = ยังวัดไม่ได้)
int  noiseSubMin     = 4095; // หน้าต่างย่อย 50 ms
int  noiseSubMax     = 0;
int  noiseSubSamples = 0;
int  noiseBestP2P    = -1;   // ค่าน้อยสุดของหน้าต่างย่อยใน 1 วินาที
unsigned long noiseSubStart = 0;
unsigned long noiseWinStart = 0;

// ---- สถิติช่วงเวลาระหว่างหยด ----------------------------------------------
#define INTERVAL_KEEP 8
uint32_t dropIntervals[INTERVAL_KEEP];
int      intervalCount = 0;
int      intervalHead  = 0;
uint32_t lastIntervalMs = 0;

// ---- บัฟเฟอร์กราฟรูปคลื่น -------------------------------------------------
uint16_t waveMin[WAVE_COLS];
uint16_t waveMax[WAVE_COLS];
bool     waveHas[WAVE_COLS];
bool     waveDrop[WAVE_COLS];
int      waveIdx        = 0;
int      bucketMin      = 4095;
int      bucketMax      = 0;
bool     bucketDrop     = false;
int      bucketSamples  = 0;
unsigned long lastBucketMs = 0;
int      plotLow = 0, plotHigh = 4095;
int      threshY = 0, hysUpY = 0, hysDownY = 0;

// ---- บัฟเฟอร์กราฟแท่งแนวโน้ม ----------------------------------------------
float    trendBin[TREND_BARS];        // ค่าเฉลี่ย gtt/min ของแต่ละช่วง
bool     trendHas[TREND_BARS];
int      trendHead      = 0;          // ช่องที่เก่าที่สุด
float    binRateSum     = 0.0f;
int      binRateSamples = 0;
float    binLiveValue   = 0.0f;
unsigned long binStartMs   = 0;
unsigned long lastRateSample = 0;
bool     trendNeedsRedraw = true;
float    chartScale     = 10.0f;      // สเกลสูงสุดของแกนตั้งที่ใช้วาดครั้งล่าสุด

// ---- โหมดจำลอง (SIM) ------------------------------------------------------
const float SIM_RATES[] = { 0.0f, 30.0f, 60.0f, 120.0f, 180.0f };   // mL/h
#define SIM_RATE_COUNT   5
#define SIM_DROP_MS      45UL          // ความกว้างของพัลส์หยด 1 หยด
#define SIM_PAUSE_EVERY  20            // ทุก ๆ 20 หยด จำลอง "หยุดไหล" 1 ครั้ง
int      simRateIdx     = 0;           // 0 = ปิด
bool     simDropActive  = false;
unsigned long simDropStartMs = 0;
unsigned long simNextDropMs  = 0;
uint32_t simDropsDone   = 0;

// ---- เสียง/ไฟ LED ---------------------------------------------------------
unsigned long buzzerBeepUntil = 0;
unsigned long ledFlashUntil   = 0;
int lastLedCode = -1;

// ---- UI ปุ่มค้าง ----------------------------------------------------------
bool isHoldUiActive  = false;
bool holdBeep3sDone  = false;
bool holdBeep5sDone  = false;
unsigned long lastHoldRenderTime = 0;

// ---- แคชข้อความ (วาดทับได้โดยไม่กะพริบ) -----------------------------------
String cacheTop1 = "", cacheTop2 = "", cacheTop3 = "";
int    cacheGtt = -999, cacheMlhr = -999;
String cacheP1a = "", cacheP1b = "", cacheP1c = "";
String cacheP2a = "", cacheP2b = "", cacheP2c = "", cacheP2kpi = "";
String cacheP3big = "", cacheP3a = "", cacheP3b = "", cacheP3c = "";
int    cacheGaugeAdc = -999;
unsigned long lastDynamicMs = 0;

// ============================================================================
// ฟังก์ชันช่วยวาดข้อความ / เสียง / ไฟ
// ============================================================================
String padCenter(const String &s, unsigned int width) {
  if (s.length() >= width) return s;
  unsigned int total = width - s.length();
  unsigned int left = total / 2;
  String out;
  out.reserve(width);
  for (unsigned int i = 0; i < left; i++) out += ' ';
  out += s;
  for (unsigned int i = 0; i < total - left; i++) out += ' ';
  return out;
}

String padRight(const String &s, unsigned int width) {
  String out = s;
  while (out.length() < width) out += ' ';
  return out;
}

void drawCenteredString(const String &str, int y, uint8_t size, uint16_t color, uint16_t bg) {
  int16_t x1, y1;
  uint16_t w, h;
  tft.setTextSize(size);
  tft.getTextBounds(str, 0, y, &x1, &y1, &w, &h);
  int x = (SCR_W - (int)w) / 2;
  if (x < 0) x = 0;
  tft.setTextColor(color, bg);
  tft.setCursor(x, y);
  tft.print(str);
}

void drawStringAt(const String &str, int x, int y, uint8_t size, uint16_t color, uint16_t bg) {
  tft.setTextSize(size);
  tft.setTextColor(color, bg);
  tft.setCursor(x, y);
  tft.print(str);
}

// เขียนข้อความชิดขวา โดยให้ตัวอักษรสุดท้ายจบที่ตำแหน่ง xRight
void drawStringRight(const String &str, int xRight, int y, uint8_t size, uint16_t color, uint16_t bg) {
  int w = str.length() * 6 * size;
  drawStringAt(str, xRight - w, y, size, color, bg);
}

void setLedColor(uint8_t r, uint8_t g, uint8_t b) {
  LED_WRITE(RGB_LED_PIN, r, g, b);
}

void beepNonBlocking(uint16_t freq, uint16_t durationMs) {
  tone(BUZZER_PIN, freq);
  buzzerBeepUntil = millis() + durationMs;
}

void modalSafeBeep(uint16_t freq, uint16_t durationMs) {
  tone(BUZZER_PIN, freq);
  delay(durationMs);
  noTone(BUZZER_PIN);
  buzzerBeepUntil = 0;
}

void soundClick() { beepNonBlocking(2600, 20); }
void soundStart() { beepNonBlocking(2000, 70); }
void soundStop()  { beepNonBlocking(1000, 140); }
void soundDrop()  { if (SOUND_ON_DROP) beepNonBlocking(3200, 12); }

void playWelcomeMelody() {
  int notes[] = { 1046, 1318, 1568, 2093 };
  for (int i = 0; i < 4; i++) {
    modalSafeBeep(notes[i], 80);
    delay(25);
  }
}

void handleBuzzerEngine(unsigned long now) {
  if (buzzerBeepUntil > 0 && now >= buzzerBeepUntil) {
    noTone(BUZZER_PIN);
    buzzerBeepUntil = 0;
  }
}

// ============================================================================
// สถานะการไหล / ตัวชี้วัดความไวของเซนเซอร์
// ============================================================================
bool simEnabled() { return simRateIdx > 0; }

int signalMargin() { return abs(valLiquid - valAir); }      // ระยะห่างสัญญาณมี/ไม่มีหยด

// อัตราส่วนสัญญาณต่อสัญญาณรบกวน ยิ่งมากยิ่งจับหยดได้แม่น
int sensitivityRatio() {
  int n = noiseP2P > 0 ? noiseP2P : 1;
  return signalMargin() / n;
}

// คำย่อสำหรับบรรทัดแคบ (ไม่เกิน 6 ตัวอักษร)
const char* sensitivityShort(uint16_t &color) {
  int r = sensitivityRatio();
  if (signalMargin() < 80) { color = COLOR_RED;    return "CALIB!"; }
  if (noiseP2P <= 0)       { color = ST77XX_LIGHTGREY; return "WAIT"; }
  if (r >= 20)             { color = COLOR_GREEN;  return "EXCEL"; }
  if (r >= 10)             { color = THEME_CYAN;   return "GOOD"; }
  if (r >= 5)              { color = COLOR_ORANGE; return "FAIR"; }
  color = COLOR_RED;       return "POOR";
}

const char* sensitivityText(uint16_t &color) {
  int r = sensitivityRatio();
  if (signalMargin() < 80) { color = COLOR_RED;    return "NEED CALIB"; }
  if (noiseP2P <= 0)       { color = ST77XX_LIGHTGREY; return "MEASURING"; }
  if (r >= 20)             { color = COLOR_GREEN;  return "EXCELLENT"; }
  if (r >= 10)             { color = THEME_CYAN;   return "GOOD"; }
  if (r >= 5)              { color = COLOR_ORANGE; return "FAIR"; }
  color = COLOR_RED;       return "POOR";
}

// เวลาที่ไม่มีหยดแล้วถือว่า "หยุดไหล" — อ้างอิงจากจังหวะหยดที่วัดได้จริง
uint32_t calcNoFlowTimeoutMs() {
  uint32_t t = 6000;
  if (currentGttMin > 0.5f) {
    uint32_t d = (uint32_t)((60000.0f / currentGttMin) * 2.5f);
    if (d > t) t = d;
  }
  if (t > 30000) t = 30000;
  return t;
}

bool isNoFlow(unsigned long now) {
  return isRunning && hasFirstDropOccurred && (now - lastDropTimestamp > calcNoFlowTimeoutMs());
}

// ค่าเฉลี่ยและความสม่ำเสมอของจังหวะหยด (CV% ยิ่งน้อยยิ่งไหลสม่ำเสมอ)
float avgIntervalMs() {
  if (intervalCount == 0) return 0.0f;
  float sum = 0;
  for (int i = 0; i < intervalCount; i++) sum += (float)dropIntervals[i];
  return sum / intervalCount;
}

int intervalCvPercent() {
  if (intervalCount < 3) return -1;
  float mean = avgIntervalMs();
  if (mean <= 0) return -1;
  float var = 0;
  for (int i = 0; i < intervalCount; i++) {
    float d = (float)dropIntervals[i] - mean;
    var += d * d;
  }
  var /= intervalCount;
  return (int)((sqrtf(var) / mean) * 100.0f);
}

void pushInterval(uint32_t ms) {
  lastIntervalMs = ms;
  dropIntervals[intervalHead] = ms;
  intervalHead = (intervalHead + 1) % INTERVAL_KEEP;
  if (intervalCount < INTERVAL_KEEP) intervalCount++;
}

// ============================================================================
// โหมดจำลองสัญญาณเซนเซอร์ (SIM) — สร้างรูปคลื่นหยดสังเคราะห์
// ส่งค่าผ่านเส้นทางตรวจจับเดียวกับของจริงทุกขั้นตอน จึงใช้ทดสอบ UI ได้เสมือนจริง
// ============================================================================
void simScheduleNextDrop(unsigned long now) {
  float rate = SIM_RATES[simRateIdx];
  float gttMin = rate * (float)DROP_FACTOR / 60.0f;
  unsigned long interval = (gttMin > 0.1f) ? (unsigned long)(60000.0f / gttMin) : 3000UL;
  long jitter = (long)(interval * 0.08f);
  if (jitter > 0) interval = (unsigned long)((long)interval + random(-jitter, jitter + 1));
  // จำลองภาวะหยุดไหลเป็นระยะ เพื่อให้เห็นกราฟตกและสถานะ NO FLOW
  if (simDropsDone > 0 && (simDropsDone % SIM_PAUSE_EVERY) == 0) interval += 8000UL;
  simNextDropMs = now + interval;
}

int simulateSensorValue(unsigned long now) {
  int span = valAir - valLiquid;                 // บวกเมื่อหยดทำให้ค่าลดลง
  int noiseAmp = abs(span) / 50;
  if (noiseAmp < 3) noiseAmp = 3;

  if (simNextDropMs == 0) simScheduleNextDrop(now);

  if (!simDropActive && now >= simNextDropMs) {
    simDropActive  = true;
    simDropStartMs = now;
  }

  float amp = 0.0f;
  if (simDropActive) {
    unsigned long e = now - simDropStartMs;
    if (e >= SIM_DROP_MS) {
      simDropActive = false;
      simDropsDone++;
      simScheduleNextDrop(now);
    } else {
      amp = sinf(PI * (float)e / (float)SIM_DROP_MS);   // รูปหยดโค้งนุ่ม ไม่ใช่สี่เหลี่ยม
    }
  }

  int v = valAir - (int)(span * amp) + (int)random(-noiseAmp, noiseAmp + 1);
  if (v < 0) v = 0;
  if (v > 4095) v = 4095;
  return v;
}

int readSensorRaw(unsigned long now) {
  if (simEnabled()) return simulateSensorValue(now);
  return analogRead(SENSOR_AO_PIN);
}

// ============================================================================
// กราฟรูปคลื่น: การแปลงค่า ADC เป็นพิกัดจอ และการวาดทีละคอลัมน์
// (วาดเฉพาะคอลัมน์ใหม่ จึงไม่แย่งเวลา SPI จนพลาดการอ่านหยด)
// ============================================================================
int adcToWaveY(int v) {
  if (plotHigh <= plotLow) return WAVE_Y + WAVE_H - 1;
  if (v < plotLow)  v = plotLow;
  if (v > plotHigh) v = plotHigh;
  long rel = (long)(v - plotLow) * (WAVE_H - 1) / (plotHigh - plotLow);
  return WAVE_Y + WAVE_H - 1 - (int)rel;
}

void refreshWaveGuideLines() {
  threshY  = adcToWaveY(dropThreshold);
  hysUpY   = adcToWaveY(dropThreshold + hysteresis);
  hysDownY = adcToWaveY(dropThreshold - hysteresis);
}

// สเกลแกนตั้งอ้างอิงค่าคาลิเบรต เพื่อให้เส้นเกณฑ์อยู่กลางจอเสมอ
void recomputePlotRange() {
  int lo = min(valLiquid, valAir);
  int hi = max(valLiquid, valAir);
  int pad = (hi - lo) / 4;
  if (pad < 100) pad = 100;
  plotLow  = lo - pad;
  plotHigh = hi + pad;
  if (plotLow < 0) plotLow = 0;
  if (plotHigh > 4095) plotHigh = 4095;
  if (plotHigh - plotLow < 100) plotHigh = plotLow + 100;
  refreshWaveGuideLines();
}

bool expandPlotRange(int v) {
  bool changed = false;
  if (v < plotLow)  { plotLow  = max(0, v - 20);    changed = true; }
  if (v > plotHigh) { plotHigh = min(4095, v + 20); changed = true; }
  if (changed) refreshWaveGuideLines();
  return changed;
}

bool onWavePage() {
  return currentState == STATE_MAIN && currentPage == PAGE_WAVE && !isHoldUiActive && !needRedraw;
}

void drawWaveColumnBase(int col) {
  int x = WAVE_X + col;
  tft.drawFastVLine(x, WAVE_Y, WAVE_H, THEME_BG);
  if ((col & 0x03) == 0) {                                  // จุดกริดแนวนอน 3 เส้น
    for (int g = 1; g <= 3; g++) tft.drawPixel(x, WAVE_Y + (WAVE_H * g) / 4, THEME_DARKBOX);
  }
  if ((col % 6) < 2) {                                      // แถบฮีสเทอรีซิส (โซนกันสัญญาณสั่น)
    tft.drawPixel(x, hysUpY,   ST77XX_DARKGREY);
    tft.drawPixel(x, hysDownY, ST77XX_DARKGREY);
  }
  if ((col % 4) < 2) tft.drawPixel(x, threshY, COLOR_ORANGE); // เส้นเกณฑ์ตัดสินว่าเป็นหยด
}

void drawWaveColumn(int col) {
  drawWaveColumnBase(col);
  if (!waveHas[col]) return;
  int yTop = adcToWaveY((int)waveMax[col]);
  int yBot = adcToWaveY((int)waveMin[col]);
  if (yBot < yTop) { int t = yTop; yTop = yBot; yBot = t; }
  tft.drawFastVLine(WAVE_X + col, yTop, yBot - yTop + 1, waveDrop[col] ? COLOR_YELLOW : THEME_CYAN);
}

void drawPulseColumn(int col) {
  int x = PULSE_X + col;
  tft.drawFastVLine(x, PULSE_Y, PULSE_H, THEME_BG);
  if (!waveHas[col]) return;
  if (waveDrop[col]) tft.drawFastVLine(x, PULSE_Y, PULSE_H, COLOR_GREEN);
  else               tft.drawPixel(x, PULSE_Y + PULSE_H - 1, ST77XX_DARKGREY);
}

void drawWaveCursor(int col) {
  drawWaveColumnBase(col);
  tft.drawFastVLine(WAVE_X + col, WAVE_Y, WAVE_H, THEME_MAROON);
  tft.drawFastVLine(PULSE_X + col, PULSE_Y, PULSE_H, THEME_MAROON);
}

void redrawWholeWave() {
  for (int c = 0; c < WAVE_COLS; c++) { drawWaveColumn(c); drawPulseColumn(c); }
  drawWaveCursor(waveIdx);
}

void clearWaveBuffer() {
  for (int c = 0; c < WAVE_COLS; c++) { waveHas[c] = false; waveDrop[c] = false; waveMin[c] = 0; waveMax[c] = 0; }
  waveIdx = 0;
  bucketMin = 4095; bucketMax = 0; bucketDrop = false; bucketSamples = 0;
}

// ============================================================================
// อ่านเซนเซอร์ + ตรวจจับหยด + เก็บข้อมูลลงบัฟเฟอร์กราฟ (เรียกทุกรอบ loop)
// ============================================================================
void sampleSensorAndDetect(unsigned long now) {
  int sensorValue = readSensorRaw(now);
  rawAdc = sensorValue;

  if (isRunning) {
    bool dropDetected, dropReleased;
    if (dropLowersAdc) {
      dropDetected = sensorValue < dropThreshold;
      dropReleased = sensorValue > (dropThreshold + hysteresis);
    } else {
      dropDetected = sensorValue > dropThreshold;
      dropReleased = sensorValue < (dropThreshold - hysteresis);
    }

    if (!dropState && dropDetected) {
      if (now - lastDebounceTime > MIN_DROP_INTERVAL) {
        dropState = true;
        totalDrops++;
        lastDebounceTime = now;

        if (rateAnchorValid) {
          unsigned long deltaMs = now - lastDropTimestamp;
          if (deltaMs > MIN_DROP_INTERVAL && deltaMs < 60000) {
            pushInterval((uint32_t)deltaMs);
            float instantGttMin  = 60000.0f / (float)deltaMs;
            float instantRateMlHr = (instantGttMin * 60.0f) / (float)DROP_FACTOR;
            if (currentFlowRate_ml_hr == 0.0f) currentFlowRate_ml_hr = instantRateMlHr;
            else currentFlowRate_ml_hr = (instantRateMlHr * 0.75f) + (currentFlowRate_ml_hr * 0.25f);
          }
        }
        rateAnchorValid = true;
        hasFirstDropOccurred = true;
        lastDropTimestamp = now;

        soundDrop();
        ledFlashUntil = now + 60;
        lastLedCode = -1;
        setLedColor(0, 90, 40);
      }
    } else if (dropState && dropReleased) {
      dropState = false;
    }
  }

  // ---- สถิติสัญญาณรบกวนของเส้นฐาน ----
  // วัดช่วงแกว่งของหน้าต่างย่อย 50 ms แล้วเลือก "ค่าน้อยที่สุด" ในรอบ 1 วินาที
  // วิธีนี้ตัดช่วงขอบขาขึ้น-ลงของหยด (ซึ่งกินเวลาไม่กี่หน้าต่างย่อย) ออกไปเองโดยอัตโนมัติ
  if (!dropState) {
    if (sensorValue < noiseSubMin) noiseSubMin = sensorValue;
    if (sensorValue > noiseSubMax) noiseSubMax = sensorValue;
    noiseSubSamples++;
  }
  if (now - noiseSubStart >= 50) {
    if (noiseSubSamples >= 5) {
      int p2p = noiseSubMax - noiseSubMin;
      if (noiseBestP2P < 0 || p2p < noiseBestP2P) noiseBestP2P = p2p;
    }
    noiseSubStart = now;
    noiseSubMin = 4095; noiseSubMax = 0; noiseSubSamples = 0;
  }
  if (now - noiseWinStart >= 1000) {
    if (noiseBestP2P >= 0) noiseP2P = noiseBestP2P;
    noiseBestP2P = -1;
    noiseWinStart = now;
  }

  // ---- เก็บค่าลงคอลัมน์กราฟ (เก็บทั้งค่าต่ำสุด/สูงสุด จึงไม่พลาดยอดแหลม) ----
  if (sensorValue < bucketMin) bucketMin = sensorValue;
  if (sensorValue > bucketMax) bucketMax = sensorValue;
  if (dropState) bucketDrop = true;
  bucketSamples++;

  if (now - lastBucketMs >= WAVE_BUCKET_MS) {
    lastBucketMs = now;
    if (bucketSamples > 0) {
      bool rangeChanged = expandPlotRange(bucketMin) | expandPlotRange(bucketMax);
      waveMin[waveIdx]  = (uint16_t)bucketMin;
      waveMax[waveIdx]  = (uint16_t)bucketMax;
      waveDrop[waveIdx] = bucketDrop;
      waveHas[waveIdx]  = true;

      if (onWavePage()) {
        if (rangeChanged) {
          redrawWholeWave();
        } else {
          drawWaveColumn(waveIdx);
          drawPulseColumn(waveIdx);
          drawWaveCursor((waveIdx + 1) % WAVE_COLS);
        }
      }
      waveIdx = (waveIdx + 1) % WAVE_COLS;
    }
    bucketMin = 4095; bucketMax = 0; bucketDrop = false; bucketSamples = 0;
  }
}

// ============================================================================
// คำนวณอัตราไหล (ลดลงตามจริงเมื่อหยดช้า) และสะสมข้อมูลกราฟแท่ง
// ============================================================================
void updateFlowMath(unsigned long now) {
  if (isRunning && hasFirstDropOccurred && currentFlowRate_ml_hr > 0.0f) {
    unsigned long elapsed = now - lastDropTimestamp;
    float expectedMs = 60000.0f / (currentFlowRate_ml_hr * DROP_FACTOR / 60.0f);
    if (elapsed > expectedMs * 1.5f && elapsed > 0) {
      float capRate = (60000.0f / (float)elapsed) * 60.0f / (float)DROP_FACTOR;
      if (capRate < currentFlowRate_ml_hr) currentFlowRate_ml_hr = capRate;
    }
    if (elapsed > calcNoFlowTimeoutMs()) currentFlowRate_ml_hr = 0.0f;
  }
  if (!isRunning) currentFlowRate_ml_hr = 0.0f;

  currentGttMin = (currentFlowRate_ml_hr * DROP_FACTOR) / 60.0f;
  totalVolumeMl = totalDrops / (float)DROP_FACTOR;
}

void updateTrendAccumulator(unsigned long now) {
  if (now - lastRateSample >= 100) {                 // เก็บตัวอย่างอัตราไหล 10 ครั้ง/วินาที
    lastRateSample = now;
    binRateSum += currentGttMin;
    binRateSamples++;
    binLiveValue = (binRateSamples > 0) ? (binRateSum / binRateSamples) : 0.0f;
  }

  if (now - binStartMs >= TREND_BIN_MS) {
    binStartMs = now;
    trendBin[trendHead] = (binRateSamples > 0) ? (binRateSum / binRateSamples) : 0.0f;
    trendHas[trendHead] = true;
    trendHead = (trendHead + 1) % TREND_BARS;
    binRateSum = 0.0f;
    binRateSamples = 0;
    binLiveValue = 0.0f;
    trendNeedsRedraw = true;
  }
}

void resetAllStatistics() {
  totalDrops = 0;
  currentFlowRate_ml_hr = 0.0f;
  currentGttMin = 0.0f;
  totalVolumeMl = 0.0f;
  hasFirstDropOccurred = false;
  rateAnchorValid = false;
  dropState = false;
  intervalCount = 0;
  intervalHead = 0;
  lastIntervalMs = 0;
  for (int i = 0; i < TREND_BARS; i++) { trendBin[i] = 0.0f; trendHas[i] = false; }
  trendHead = 0;
  binRateSum = 0.0f; binRateSamples = 0; binLiveValue = 0.0f;
  binStartMs = millis();
  trendNeedsRedraw = true;
  clearWaveBuffer();
  needRedraw = true;
  beepNonBlocking(2400, 120);
}

// ============================================================================
// แถบสถานะด้านบน (ใช้ร่วมกันทุกหน้า)
//   ซ้าย = สถานะการนับ / กลาง = ชื่อหน้า / ขวา = โหมดจำลองหรือ Drop factor
// ============================================================================
String runStateText(unsigned long now) {
  if (!isRunning)    return "HOLD";
  if (isNoFlow(now)) return "NOFLOW";
  return "RUN";
}

uint16_t runStateColor(unsigned long now) {
  if (!isRunning)    return COLOR_YELLOW;
  if (isNoFlow(now)) return COLOR_RED;
  return COLOR_GREEN;
}

const char* pageTitle() {
  switch (currentPage) {
    case PAGE_WAVE:   return "WAVEFORM";
    case PAGE_TREND:  return "FLOW TREND";
    default:          return "SENSOR LAB";
  }
}

String badgeText() {
  if (simEnabled()) return "SIM " + String((int)SIM_RATES[simRateIdx]);
  return "DF" + String(DROP_FACTOR);
}

void drawTopBar(unsigned long now, bool forceRedraw) {
  if (forceRedraw) {
    tft.fillRect(0, 0, SCR_W, TOPBAR_H, THEME_NAVY);
    tft.drawFastHLine(0, TOPBAR_H, SCR_W, THEME_CYAN);
    cacheTop1 = ""; cacheTop2 = ""; cacheTop3 = "";
  }

  String s1 = padRight(runStateText(now), 6);
  if (s1 != cacheTop1) {
    cacheTop1 = s1;
    drawStringAt(s1, 5, 7, 1, runStateColor(now), THEME_NAVY);
  }

  String s2 = pageTitle();
  if (s2 != cacheTop2) {
    cacheTop2 = s2;
    tft.fillRect(46, 5, 78, 12, THEME_NAVY);
    drawCenteredString(s2, 7, 1, COLOR_WHITE, THEME_NAVY);
  }

  String s3 = badgeText();
  if (s3 != cacheTop3) {
    cacheTop3 = s3;
    tft.fillRect(120, 5, 48, 12, THEME_NAVY);
    drawStringRight(s3, 167, 7, 1, simEnabled() ? THEME_PINK : THEME_SKYBLUE, THEME_NAVY);
  }
}

void drawFooterNav() {
  tft.fillRect(0, FOOTER_Y - 2, SCR_W, 20, THEME_BG);
  String nav;
  if (currentPage == PAGE_WAVE)       nav = "[1]Wave 2:Trend 3:Sensor";
  else if (currentPage == PAGE_TREND) nav = "1:Wave [2]Trend 3:Sensor";
  else                                nav = "1:Wave 2:Trend [3]Sensor";
  drawCenteredString(nav, FOOTER_Y, 1, THEME_SKYBLUE, THEME_BG);
}

// ============================================================================
// หน้า 1: WAVEFORM — รูปคลื่นสัญญาณเซนเซอร์ + แถบพัลส์การนับหยด
// ============================================================================
void drawWavePageFramework() {
  tft.fillScreen(THEME_BG);
  drawTopBar(millis(), true);

  // กล่องตัวเลขอัตราไหล
  tft.fillRoundRect(6, 26, 160, 48, 6, THEME_DARKBOX);
  tft.drawRoundRect(6, 26, 160, 48, 6, THEME_SKYBLUE);
  drawStringAt("FLOW", 12, 30, 1, THEME_SKYBLUE, THEME_DARKBOX);
  drawStringAt("gtt/min", 12, 62, 1, THEME_PINK, THEME_DARKBOX);
  drawStringAt("mL/h", 96, 62, 1, THEME_PINK, THEME_DARKBOX);

  // กล่องกราฟรูปคลื่น
  tft.fillRoundRect(6, 78, 160, 120, 6, THEME_BG);
  tft.drawRoundRect(6, 78, 160, 120, 6, THEME_CYAN);
  drawStringAt("SENSOR ADC", 12, 82, 1, THEME_CYAN, THEME_BG);
  drawStringRight("3 s window", 162, 82, 1, ST77XX_DARKGREY, THEME_BG);

  // กล่องแถบพัลส์
  tft.fillRoundRect(6, 202, 160, 34, 6, THEME_BG);
  tft.drawRoundRect(6, 202, 160, 34, 6, COLOR_GREEN);
  drawStringAt("DROP PULSE", 12, 205, 1, COLOR_GREEN, THEME_BG);

  redrawWholeWave();
  drawFooterNav();

  cacheGtt = -999; cacheMlhr = -999;
  cacheP1a = ""; cacheP1b = ""; cacheP1c = "";
}

void drawWavePageDynamic(unsigned long now) {
  drawTopBar(now, false);

  int gtt  = (int)(currentGttMin + 0.5f);
  int mlhr = (int)(currentFlowRate_ml_hr + 0.5f);
  if (gtt != cacheGtt || mlhr != cacheMlhr) {
    cacheGtt = gtt; cacheMlhr = mlhr;
    char buf[8];
    snprintf(buf, sizeof(buf), "%3d", gtt);
    drawStringAt(String(buf), 10, 38, 3, isRunning ? COLOR_WHITE : COLOR_YELLOW, THEME_DARKBOX);
    snprintf(buf, sizeof(buf), "%4d", mlhr);
    drawStringAt(String(buf), 94, 42, 2, isRunning ? COLOR_WHITE : COLOR_YELLOW, THEME_DARKBOX);
  }

  char line[40];
  snprintf(line, sizeof(line), "ADC %4d  TH %4d  HYS %3d", rawAdc, dropThreshold, hysteresis);
  String l1 = padCenter(String(line), 28);
  if (l1 != cacheP1a) { cacheP1a = l1; drawCenteredString(l1, 244, 1, COLOR_WHITE, THEME_BG); }

  float lastSec = (lastIntervalMs > 0) ? (lastIntervalMs / 1000.0f) : 0.0f;
  snprintf(line, sizeof(line), "Last %4.2f s   Drops %lu", lastSec, (unsigned long)totalDrops);
  String l2 = padCenter(String(line), 28);
  if (l2 != cacheP1b) { cacheP1b = l2; drawCenteredString(l2, 260, 1, THEME_SKYBLUE, THEME_BG); }

  uint16_t qColor;
  const char* q = sensitivityShort(qColor);
  snprintf(line, sizeof(line), "Noise %3d  S/N %-3d %s", noiseP2P, sensitivityRatio(), q);
  String l3 = padCenter(String(line), 28);
  if (l3 != cacheP1c) { cacheP1c = l3; drawCenteredString(l3, 276, 1, qColor, THEME_BG); }
}

// ============================================================================
// หน้า 2: FLOW TREND — กราฟแท่งแนวโน้มอัตราไหลย้อนหลัง 60 วินาที
// ============================================================================
float trendValueAt(int i) {                 // i = 0 คือแท่งซ้ายสุด (เก่าสุด)
  int idx = (trendHead + i) % TREND_BARS;
  return trendHas[idx] ? trendBin[idx] : 0.0f;
}

bool trendValidAt(int i) {
  int idx = (trendHead + i) % TREND_BARS;
  return trendHas[idx];
}

float niceScale(float v) {
  float s = 10.0f;
  while (s < v && s < 300.0f) s += (s < 50.0f) ? 10.0f : 25.0f;
  return s;
}

void drawTrendChart() {
  const int chartH = CHART_BOT - CHART_TOP;
  const int chartW = TREND_BARS * CHART_BAR_W;

  float maxV = 1.0f, sum = 0.0f;
  int valid = 0;
  for (int i = 0; i < TREND_BARS - 1; i++) {
    if (!trendValidAt(i + 1)) continue;      // แท่งขวาสุดสงวนไว้ให้ค่าที่กำลังวัดอยู่
    float v = trendValueAt(i + 1);
    if (v > maxV) maxV = v;
    sum += v; valid++;
  }
  if (binLiveValue > maxV) maxV = binLiveValue;
  float scale = niceScale(maxV * 1.15f);
  chartScale = scale;
  float avg = (valid > 0) ? (sum / valid) : 0.0f;

  tft.fillRect(CHART_X - 3, CHART_TOP - 10, chartW + 6, chartH + 24, THEME_BG);

  // เส้นกริดแนวนอน 3 เส้น
  for (int g = 1; g <= 3; g++) {
    int gy = CHART_BOT - (chartH * g) / 4;
    for (int x = CHART_X; x < CHART_X + chartW; x += 4) tft.drawPixel(x, gy, THEME_DARKBOX);
  }

  // แท่งกราฟ: 29 แท่งที่ปิดช่วงแล้ว + แท่งขวาสุดคือช่วงที่กำลังวัด
  for (int i = 0; i < TREND_BARS; i++) {
    bool isLive = (i == TREND_BARS - 1);
    float v = isLive ? binLiveValue : trendValueAt(i + 1);
    bool has = isLive ? true : trendValidAt(i + 1);
    int h = (int)((v / scale) * chartH);
    if (h > chartH) h = chartH;
    int x = CHART_X + i * CHART_BAR_W;
    if (!has) {
      tft.drawFastHLine(x, CHART_BOT, CHART_BAR_W - 1, THEME_DARKBOX);
    } else if (h <= 1) {
      tft.drawFastHLine(x, CHART_BOT, CHART_BAR_W - 1, COLOR_RED);   // ช่วงที่ไม่มีการไหล
    } else {
      uint16_t c = isLive ? THEME_PINK : THEME_CYAN;
      tft.fillRect(x, CHART_BOT - h, CHART_BAR_W - 1, h, c);
    }
  }

  // เส้นค่าเฉลี่ยของหน้าต่างเวลา
  if (avg > 0.5f) {
    int ay = CHART_BOT - (int)((avg / scale) * chartH);
    if (ay < CHART_TOP) ay = CHART_TOP;
    for (int x = CHART_X; x < CHART_X + chartW; x += 6) tft.drawFastHLine(x, ay, 3, COLOR_YELLOW);
  }

  tft.drawFastHLine(CHART_X - 2, CHART_BOT + 1, chartW + 4, ST77XX_LIGHTGREY);
  drawStringAt(padRight(String((int)scale), 4), CHART_X, CHART_TOP - 10, 1, ST77XX_LIGHTGREY, THEME_BG);
  drawStringAt("-60s", CHART_X, CHART_BOT + 5, 1, ST77XX_DARKGREY, THEME_BG);
  drawStringRight("now", CHART_X + chartW, CHART_BOT + 5, 1, ST77XX_DARKGREY, THEME_BG);
  trendNeedsRedraw = false;
}

// วาดใหม่เฉพาะแท่งขวาสุด (ช่วงที่กำลังวัดอยู่) เพื่อให้กราฟขยับลื่นโดยไม่กะพริบทั้งภาพ
void drawTrendLiveBar() {
  if (binLiveValue > chartScale) { drawTrendChart(); return; }
  const int chartH = CHART_BOT - CHART_TOP;
  int x = CHART_X + (TREND_BARS - 1) * CHART_BAR_W;
  int h = (int)((binLiveValue / chartScale) * chartH);
  if (h > chartH) h = chartH;
  tft.fillRect(x, CHART_TOP, CHART_BAR_W - 1, chartH, THEME_BG);
  for (int g = 1; g <= 3; g++) {
    int gy = CHART_BOT - (chartH * g) / 4;
    if (((x - CHART_X) & 0x03) == 0) tft.drawPixel(x, gy, THEME_DARKBOX);
  }
  if (h <= 1) tft.drawFastHLine(x, CHART_BOT, CHART_BAR_W - 1, COLOR_RED);
  else        tft.fillRect(x, CHART_BOT - h, CHART_BAR_W - 1, h, THEME_PINK);
}

void drawTrendPageFramework() {
  tft.fillScreen(THEME_BG);
  drawTopBar(millis(), true);

  tft.fillRoundRect(6, 26, 160, 54, 6, THEME_DARKBOX);
  tft.drawRoundRect(6, 26, 160, 54, 6, THEME_PINK);
  drawStringAt("gtt/min", 12, 62, 1, THEME_PINK, THEME_DARKBOX);

  tft.fillRoundRect(6, 86, 160, 152, 6, THEME_BG);
  tft.drawRoundRect(6, 86, 160, 152, 6, THEME_CYAN);
  drawStringAt("TREND", 12, 90, 1, THEME_CYAN, THEME_BG);
  drawStringRight("2 s / bar", 162, 90, 1, ST77XX_DARKGREY, THEME_BG);

  drawTrendChart();
  drawFooterNav();

  cacheGtt = -999; cacheP2kpi = ""; cacheP2a = ""; cacheP2b = ""; cacheP2c = "";
}

void drawTrendPageDynamic(unsigned long now) {
  drawTopBar(now, false);

  int gtt = (int)(currentGttMin + 0.5f);
  if (gtt != cacheGtt) {
    cacheGtt = gtt;
    char buf[8];
    snprintf(buf, sizeof(buf), "%3d", gtt);
    drawStringAt(String(buf), 12, 34, 3, isRunning ? COLOR_WHITE : COLOR_YELLOW, THEME_DARKBOX);
  }

  char kpi[40];
  snprintf(kpi, sizeof(kpi), "%4d mL/h", (int)(currentFlowRate_ml_hr + 0.5f));
  String k1 = String(kpi);
  snprintf(kpi, sizeof(kpi), "%4d drops", (int)totalDrops);
  String k2 = String(kpi);
  snprintf(kpi, sizeof(kpi), "%5.1f mL", totalVolumeMl);
  String k3 = String(kpi);
  String kAll = k1 + k2 + k3;
  if (kAll != cacheP2kpi) {
    cacheP2kpi = kAll;
    drawStringAt(padRight(k1, 10), 88, 34, 1, COLOR_WHITE,   THEME_DARKBOX);
    drawStringAt(padRight(k2, 10), 88, 48, 1, THEME_SKYBLUE, THEME_DARKBOX);
    drawStringAt(padRight(k3, 10), 88, 62, 1, COLOR_GREEN,   THEME_DARKBOX);
  }

  // สรุปหน้าต่างเวลา 60 วินาที
  float mx = 0.0f, mn = 9999.0f, sum = 0.0f;
  int valid = 0;
  for (int i = 1; i < TREND_BARS; i++) {
    if (!trendValidAt(i)) continue;
    float v = trendValueAt(i);
    if (v > mx) mx = v;
    if (v < mn) mn = v;
    sum += v; valid++;
  }
  if (valid == 0) { mn = 0.0f; mx = 0.0f; }
  float avg = (valid > 0) ? sum / valid : 0.0f;

  char line[40];
  snprintf(line, sizeof(line), "Max %3d  Min %3d  Avg %3d", (int)(mx + 0.5f), (int)(mn + 0.5f), (int)(avg + 0.5f));
  String l1 = padCenter(String(line), 28);
  if (l1 != cacheP2a) { cacheP2a = l1; drawCenteredString(l1, 246, 1, COLOR_WHITE, THEME_BG); }

  int cv = intervalCvPercent();
  float ai = avgIntervalMs() / 1000.0f;
  if (cv >= 0) snprintf(line, sizeof(line), "Interval %4.2f s  CV %2d %%", ai, cv);
  else         snprintf(line, sizeof(line), "Interval --      CV --");
  String l2 = padCenter(String(line), 28);
  if (l2 != cacheP2b) {
    cacheP2b = l2;
    uint16_t c = (cv < 0) ? ST77XX_LIGHTGREY : (cv <= 10 ? COLOR_GREEN : (cv <= 25 ? COLOR_ORANGE : COLOR_RED));
    drawCenteredString(l2, 262, 1, c, THEME_BG);
  }

  String l3 = padCenter(isNoFlow(now) ? String("** NO FLOW DETECTED **")
                                      : (isRunning ? String("Counting...") : String("Paused (2 clicks = run)")), 28);
  if (l3 != cacheP2c) {
    cacheP2c = l3;
    drawCenteredString(l3, 278, 1, isNoFlow(now) ? COLOR_RED : (isRunning ? COLOR_GREEN : COLOR_YELLOW), THEME_BG);
  }

  if (trendNeedsRedraw) drawTrendChart();
  else                  drawTrendLiveBar();
}

// ============================================================================
// หน้า 3: SENSOR LAB — ค่าคาลิเบรต เกจสัญญาณสด และคะแนนความไวของเซนเซอร์
// ============================================================================
void drawSensorGauge(int adcValue) {
  const int gx = 16, gy = 84, gw = 140, gh = 14;
  int lo = min(valLiquid, valAir);
  int hi = max(valLiquid, valAir);
  int pad = (hi - lo) / 5;
  if (pad < 50) pad = 50;
  int lowEnd = lo - pad, highEnd = hi + pad;
  if (highEnd <= lowEnd) highEnd = lowEnd + 100;

  tft.fillRect(gx, gy, gw, gh, THEME_BG);
  tft.drawRect(gx - 1, gy - 1, gw + 2, gh + 2, ST77XX_DARKGREY);

  // โซนฮีสเทอรีซิสรอบเส้นเกณฑ์ตัดสิน
  int hx1 = gx + (long)(dropThreshold - hysteresis - lowEnd) * gw / (highEnd - lowEnd);
  int hx2 = gx + (long)(dropThreshold + hysteresis - lowEnd) * gw / (highEnd - lowEnd);
  if (hx1 < gx) hx1 = gx;
  if (hx2 > gx + gw) hx2 = gx + gw;
  if (hx2 > hx1) tft.fillRect(hx1, gy, hx2 - hx1, gh, THEME_DARKBOX);

  // เครื่องหมายค่าอ้างอิงและเส้นเกณฑ์
  int lx = gx + (long)(valLiquid - lowEnd) * gw / (highEnd - lowEnd);
  int ax = gx + (long)(valAir    - lowEnd) * gw / (highEnd - lowEnd);
  int tx = gx + (long)(dropThreshold - lowEnd) * gw / (highEnd - lowEnd);
  tft.drawFastVLine(lx, gy, gh, THEME_CYAN);
  tft.drawFastVLine(ax, gy, gh, THEME_SKYBLUE);
  tft.drawFastVLine(tx, gy, gh, COLOR_ORANGE);

  // ตำแหน่งค่าที่อ่านได้ตอนนี้
  int px = gx + (long)(adcValue - lowEnd) * gw / (highEnd - lowEnd);
  if (px < gx) px = gx;
  if (px > gx + gw - 3) px = gx + gw - 3;
  tft.fillRect(px - 1, gy - 3, 3, gh + 6, dropState ? COLOR_YELLOW : COLOR_WHITE);
}

void drawSensorPageFramework() {
  tft.fillScreen(THEME_BG);
  drawTopBar(millis(), true);

  tft.fillRoundRect(6, 26, 160, 92, 6, THEME_DARKBOX);
  tft.drawRoundRect(6, 26, 160, 92, 6, THEME_SKYBLUE);
  drawStringAt("LIVE SENSOR ADC", 12, 30, 1, THEME_SKYBLUE, THEME_DARKBOX);
  drawStringAt("LIQ", 16, 102, 1, THEME_CYAN, THEME_DARKBOX);
  drawCenteredString("TH", 102, 1, COLOR_ORANGE, THEME_DARKBOX);
  drawStringRight("AIR", 156, 102, 1, THEME_SKYBLUE, THEME_DARKBOX);

  tft.fillRoundRect(6, 124, 160, 86, 6, THEME_NAVY);
  tft.drawRoundRect(6, 124, 160, 86, 6, THEME_PINK);
  drawStringAt("CALIBRATION", 12, 128, 1, THEME_PINK, THEME_NAVY);

  tft.fillRoundRect(6, 216, 160, 76, 6, THEME_DARKBOX);
  tft.drawRoundRect(6, 216, 160, 76, 6, COLOR_GREEN);
  drawStringAt("SENSITIVITY", 12, 220, 1, COLOR_GREEN, THEME_DARKBOX);

  drawFooterNav();
  cacheP3big = ""; cacheP3a = ""; cacheP3b = ""; cacheP3c = ""; cacheGaugeAdc = -999;
}

void drawSensorPageDynamic(unsigned long now) {
  drawTopBar(now, false);

  char buf[40];
  snprintf(buf, sizeof(buf), "%4d", rawAdc);
  String big = String(buf);
  if (big != cacheP3big) {
    cacheP3big = big;
    drawCenteredString(big, 46, 4, dropState ? COLOR_YELLOW : COLOR_WHITE, THEME_DARKBOX);
  }
  if (abs(rawAdc - cacheGaugeAdc) >= 2) {
    cacheGaugeAdc = rawAdc;
    drawSensorGauge(rawAdc);
  }

  // กล่องค่าคาลิเบรต
  char l1[40], l2[40], l3[40];
  snprintf(l1, sizeof(l1), "Liquid %4d   Air %4d", valLiquid, valAir);
  snprintf(l2, sizeof(l2), "Thresh %4d   Hys %3d", dropThreshold, hysteresis);
  snprintf(l3, sizeof(l3), "Drop = %s ADC", dropLowersAdc ? "LOW " : "HIGH");
  String block = String(l1) + String(l2) + String(l3);
  if (block != cacheP3a) {
    cacheP3a = block;
    drawStringAt(padRight(String(l1), 24), 12, 146, 1, COLOR_WHITE,   THEME_NAVY);
    drawStringAt(padRight(String(l2), 24), 12, 162, 1, COLOR_WHITE,   THEME_NAVY);
    drawStringAt(padRight(String(l3), 24), 12, 178, 1, THEME_SKYBLUE, THEME_NAVY);
    snprintf(buf, sizeof(buf), "Signal margin %4d ADC", signalMargin());
    drawStringAt(padRight(String(buf), 24), 12, 194, 1,
                 signalMargin() >= 300 ? COLOR_GREEN : (signalMargin() >= 80 ? COLOR_ORANGE : COLOR_RED), THEME_NAVY);
  }

  // กล่องความไว
  uint16_t qColor;
  const char* q = sensitivityText(qColor);
  snprintf(buf, sizeof(buf), "Noise p-p %3d ADC", noiseP2P);
  String s1 = padRight(String(buf), 24);
  if (s1 != cacheP3b) { cacheP3b = s1; drawStringAt(s1, 12, 238, 1, COLOR_WHITE, THEME_DARKBOX); }

  snprintf(buf, sizeof(buf), "Ratio x%-4d %s", sensitivityRatio(), q);
  String s2 = padRight(String(buf), 24);
  if (s2 != cacheP3c) {
    cacheP3c = s2;
    drawStringAt(s2, 12, 254, 1, qColor, THEME_DARKBOX);
    float lastSec = (lastIntervalMs > 0) ? (lastIntervalMs / 1000.0f) : 0.0f;
    snprintf(buf, sizeof(buf), "Drops %-5lu Last %4.2f s", (unsigned long)totalDrops, lastSec);
    drawStringAt(padRight(String(buf), 24), 12, 270, 1, THEME_SKYBLUE, THEME_DARKBOX);
  }
}

// ============================================================================
// หน้าวิธีใช้ / ผู้พัฒนา (กดปุ่มค้าง 5 วินาที)
// ============================================================================
void drawAboutScreen() {
  tft.fillScreen(THEME_MAROON);
  tft.drawRoundRect(4, 4, 164, 312, 8, COLOR_WHITE);
  tft.drawRoundRect(6, 6, 160, 308, 6, COLOR_ORANGE);
  tft.fillRoundRect(12, 14, 148, 30, 6, 0x3800);
  drawCenteredString("HOW TO USE", 22, 2, COLOR_YELLOW, 0x3800);

  drawStringAt("1 click  : change page",   14, 56,  1, COLOR_WHITE,   THEME_MAROON);
  drawStringAt("2 clicks : RUN / HOLD",    14, 72,  1, COLOR_WHITE,   THEME_MAROON);
  drawStringAt("3 clicks : reset stats",   14, 88,  1, COLOR_WHITE,   THEME_MAROON);
  drawStringAt("4 clicks : SIM mode/rate", 14, 104, 1, THEME_SKYBLUE, THEME_MAROON);
  drawStringAt("hold 3 s : CALIBRATION",   14, 120, 1, COLOR_YELLOW,  THEME_MAROON);
  drawStringAt("hold 5 s : this screen",   14, 136, 1, ST77XX_LIGHTGREY, THEME_MAROON);

  tft.drawFastHLine(16, 154, 140, COLOR_ORANGE);
  drawCenteredString("PAGES", 162, 1, COLOR_ORANGE, THEME_MAROON);
  drawStringAt("1 WAVE  : live ADC signal",  14, 178, 1, COLOR_WHITE, THEME_MAROON);
  drawStringAt("2 TREND : flow bar graph",   14, 194, 1, COLOR_WHITE, THEME_MAROON);
  drawStringAt("3 SENSOR: calib + quality",  14, 210, 1, COLOR_WHITE, THEME_MAROON);

  tft.drawFastHLine(16, 228, 140, COLOR_ORANGE);
  drawCenteredString("Smart IV Alert - Flow Lab", 236, 1, COLOR_WHITE, THEME_MAROON);
  drawCenteredString("Developer", 252, 1, COLOR_ORANGE, THEME_MAROON);
  drawCenteredString("Kittiphan Rattanakorn", 266, 1, COLOR_YELLOW, THEME_MAROON);
  drawCenteredString("BCN Phrae Innovation", 280, 1, THEME_SKYBLUE, THEME_MAROON);

  tft.fillRoundRect(14, 294, 144, 18, 4, 0x3800);
  drawCenteredString("FW v" APP_VERSION "  [CLICK=BACK]", 299, 1, COLOR_WHITE, 0x3800);
  needRedraw = false;
}

// ============================================================================
// HUD ขณะกดปุ่มค้าง
// ============================================================================
void drawHoldProgressHUD(unsigned long holdDur) {
  int boxX = 6, boxY = 110, boxW = 160, boxH = 100;
  tft.fillRoundRect(boxX, boxY, boxW, boxH, 8, THEME_NAVY);
  tft.drawRoundRect(boxX, boxY, boxW, boxH, 8, THEME_CYAN);

  int barX = boxX + 15, barY = boxY + 48, barW = 130, barH = 12;

  if (holdDur < 3000) {
    int fill = map((long)holdDur, 400, 3000, 0, barW);
    if (fill < 0) fill = 0;
    if (fill > barW) fill = barW;
    drawCenteredString("BUTTON HOLDING", boxY + 12, 1, THEME_SKYBLUE, THEME_NAVY);
    drawCenteredString("-> CALIBRATE (3s)", boxY + 28, 1, COLOR_YELLOW, THEME_NAVY);
    tft.drawRect(barX, barY, barW, barH, COLOR_WHITE);
    tft.fillRect(barX + 1, barY + 1, fill, barH - 2, THEME_PINK);
    tft.fillRect(barX + 1 + fill, barY + 1, barW - fill - 2, barH - 2, THEME_NAVY);
    drawCenteredString("Release <3s = Cancel", boxY + 82, 1, ST77XX_LIGHTGREY, THEME_NAVY);
  } else if (holdDur < 5000) {
    int fill = map((long)holdDur, 3000, 5000, 0, barW);
    if (fill < 0) fill = 0;
    if (fill > barW) fill = barW;
    drawCenteredString("RELEASE = CALIB", boxY + 12, 1, COLOR_GREEN, THEME_NAVY);
    drawCenteredString("-> HOW TO USE (5s)", boxY + 28, 1, THEME_PINK, THEME_NAVY);
    tft.drawRect(barX, barY, barW, barH, COLOR_WHITE);
    tft.fillRect(barX + 1, barY + 1, fill, barH - 2, THEME_CYAN);
    tft.fillRect(barX + 1 + fill, barY + 1, barW - fill - 2, barH - 2, THEME_NAVY);
    drawCenteredString("Release now = Calib", boxY + 82, 1, ST77XX_LIGHTGREY, THEME_NAVY);
  } else {
    drawCenteredString(">> HOW TO USE <<", boxY + 14, 1, COLOR_GREEN, THEME_NAVY);
    drawCenteredString("RELEASE TO VIEW", boxY + 30, 1, COLOR_YELLOW, THEME_NAVY);
    tft.drawRect(barX, barY, barW, barH, COLOR_WHITE);
    tft.fillRect(barX + 1, barY + 1, barW - 2, barH - 2, COLOR_GREEN);
  }
}

// ============================================================================
// Calibration Wizard — วัดค่าอ้างอิง 2 จุด แล้วคำนวณเกณฑ์ตัดสิน/ฮีสเทอรีซิส
// (โหมดโมดัล: หยุดงานอื่นชั่วคราวจนกว่าจะทำเสร็จ)
// ============================================================================
int sampleSensorAveragePrecise(int samples = 50) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(SENSOR_AO_PIN);
    delayMicroseconds(500);
  }
  return (int)(sum / samples);
}

// รอผู้ใช้กดปุ่มพร้อมแสดงค่า ADC สดและช่วงแกว่ง เพื่อให้ตั้งตำแหน่งเซนเซอร์ได้นิ่งก่อนเก็บค่า
int waitAndMonitorSensorStep(const char* stepTitle, const char* stepDescTh, const char* stepDescEn, uint16_t boxColor) {
  noTone(BUZZER_PIN);
  buzzerBeepUntil = 0;

  tft.fillRoundRect(6, 36, 160, 152, 8, THEME_DARKBOX);
  tft.drawRoundRect(6, 36, 160, 152, 8, boxColor);
  drawCenteredString(stepTitle, 44, 1, COLOR_YELLOW, THEME_DARKBOX);
  drawCenteredString(stepDescEn, 60, 2, COLOR_WHITE, THEME_DARKBOX);
  drawCenteredString(stepDescTh, 82, 1, THEME_PINK, THEME_DARKBOX);

  tft.fillRoundRect(20, 102, 132, 44, 6, THEME_NAVY);
  tft.drawRoundRect(20, 102, 132, 44, 6, boxColor);
  drawCenteredString("SENSOR LIVE ADC", 106, 1, THEME_SKYBLUE, THEME_NAVY);
  drawCenteredString("Hold steady, then click", 170, 1, COLOR_WHITE, THEME_DARKBOX);

  unsigned long lastAdcUpdate = 0, winStart = millis();
  int lastVal = -1, wMin = 4095, wMax = 0, shownSpread = -1;

  while (digitalRead(BTN_PIN) == HIGH) {
    int liveAdc = analogRead(SENSOR_AO_PIN);
    if (liveAdc < wMin) wMin = liveAdc;
    if (liveAdc > wMax) wMax = liveAdc;

    if (millis() - lastAdcUpdate >= 60) {
      lastAdcUpdate = millis();
      if (liveAdc != lastVal) {
        lastVal = liveAdc;
        tft.fillRect(24, 120, 124, 20, THEME_NAVY);
        drawCenteredString(String(liveAdc), 120, 2, COLOR_WHITE, THEME_NAVY);
      }
    }
    if (millis() - winStart >= 800) {          // รายงานความนิ่งของสัญญาณทุก 0.8 วินาที
      winStart = millis();
      int spread = wMax - wMin;
      if (spread != shownSpread) {
        shownSpread = spread;
        String txt = "Stability +/- " + String(spread) + " ADC";
        drawCenteredString(padCenter(txt, 26), 152, 1,
                           (spread <= 20) ? COLOR_GREEN : (spread <= 60 ? COLOR_ORANGE : COLOR_RED), THEME_DARKBOX);
      }
      wMin = 4095; wMax = 0;
    }
    delay(5);
  }

  modalSafeBeep(2400, 40);
  while (digitalRead(BTN_PIN) == LOW) delay(10);
  delay(60);

  tft.fillRect(20, 102, 132, 44, THEME_NAVY);
  drawCenteredString("SAMPLING...", 118, 1, COLOR_YELLOW, THEME_NAVY);
  int avgVal = sampleSensorAveragePrecise(50);
  modalSafeBeep(1800, 60);
  return avgVal;
}

void executeCalibrationWizard() {
  noTone(BUZZER_PIN);
  buzzerBeepUntil = 0;

  tft.fillScreen(THEME_BG);
  tft.fillRect(0, 0, SCR_W, 28, THEME_PINK);
  drawCenteredString("CALIBRATION WIZARD", 8, 1, COLOR_WHITE, THEME_PINK);

  // โหมดจำลองใช้สัญญาณสังเคราะห์ จึงคาลิเบรตไม่ได้ ต้องปิดก่อน
  if (simEnabled()) {
    tft.fillRoundRect(6, 60, 160, 140, 8, THEME_NAVY);
    tft.drawRoundRect(6, 60, 160, 140, 8, COLOR_RED);
    drawCenteredString("SIM MODE IS ON", 80, 2, COLOR_RED, THEME_NAVY);
    drawCenteredString("Calibration needs", 116, 1, COLOR_WHITE, THEME_NAVY);
    drawCenteredString("the real sensor.", 132, 1, COLOR_WHITE, THEME_NAVY);
    drawCenteredString("Turn SIM off first", 152, 1, THEME_PINK, THEME_NAVY);
    drawCenteredString("4 clicks = SIM off", 172, 1, COLOR_YELLOW, THEME_NAVY);
    modalSafeBeep(800, 200);
    delay(2200);
    needRedraw = true;
    return;
  }

  int newLiquid = waitAndMonitorSensorStep("STEP 1 OF 2", "Chamber shows liquid", "FILL LIQUID", THEME_SKYBLUE);
  int newAir    = waitAndMonitorSensorStep("STEP 2 OF 2", "Chamber shows air only", "EMPTY CHAMBER", THEME_PINK);

  int diff = abs(newLiquid - newAir);
  tft.fillRoundRect(6, 196, 160, 116, 8, THEME_NAVY);
  tft.drawRoundRect(6, 196, 160, 116, 8, (diff >= 80) ? COLOR_GREEN : COLOR_RED);

  if (diff >= 80) {
    valLiquid     = newLiquid;
    valAir        = newAir;
    dropThreshold = (valLiquid + valAir) / 2;
    dropLowersAdc = (valLiquid < valAir);      // รองรับเซนเซอร์ทั้งสองขั้ว
    hysteresis    = (int)(diff * 0.20f);
    if (hysteresis < 60)  hysteresis = 60;
    if (hysteresis > 300) hysteresis = 300;

    labPrefs.begin("st_cal", false);
    labPrefs.putInt("v_liq", valLiquid);
    labPrefs.putInt("v_air", valAir);
    labPrefs.putInt("th", dropThreshold);
    labPrefs.putInt("hys", hysteresis);
    labPrefs.end();

    recomputePlotRange();
    clearWaveBuffer();

    drawCenteredString("CALIBRATION OK!", 206, 1, COLOR_GREEN, THEME_NAVY);
    drawCenteredString("SIGNAL MARGIN", 222, 1, THEME_SKYBLUE, THEME_NAVY);
    drawCenteredString(String(diff) + " ADC", 234, 2, COLOR_WHITE, THEME_NAVY);
    drawCenteredString("Thresh " + String(dropThreshold) + " / Hys " + String(hysteresis), 258, 1, THEME_SKYBLUE, THEME_NAVY);
    drawCenteredString(dropLowersAdc ? "Mode: drop = LOW ADC" : "Mode: drop = HIGH ADC", 274, 1, COLOR_WHITE, THEME_NAVY);
    drawCenteredString("Saved to flash", 292, 1, ST77XX_LIGHTGREY, THEME_NAVY);
    modalSafeBeep(2000, 100);
    delay(100);
    modalSafeBeep(2500, 150);
    delay(1800);
  } else {
    drawCenteredString("CALIB FAILED", 210, 2, COLOR_RED, THEME_NAVY);
    drawCenteredString("Margin " + String(diff) + " ADC (min 80)", 238, 1, COLOR_WHITE, THEME_NAVY);
    drawCenteredString("Check sensor position", 260, 1, COLOR_YELLOW, THEME_NAVY);
    drawCenteredString("Keep previous values", 280, 1, COLOR_WHITE, THEME_NAVY);
    modalSafeBeep(800, 200);
    delay(1800);
  }

  dropState = false;
  lastDebounceTime = millis();
  needRedraw = true;
}

// ============================================================================
// ปุ่มกดเดียว: คลิก 1-4 ครั้ง และกดค้าง
// ============================================================================
void cycleSimulationMode() {
  simRateIdx = (simRateIdx + 1) % SIM_RATE_COUNT;
  simDropActive = false;
  simDropsDone  = 0;
  simNextDropMs = 0;
  dropState = false;
  clearWaveBuffer();
  needRedraw = true;
  beepNonBlocking(simEnabled() ? 2400 : 1200, 90);
}

void handleButton() {
  static unsigned long btnPressStart = 0;
  static bool isHolding = false;
  static int clickCount = 0;
  static unsigned long lastReleaseTime = 0;

  bool reading = (digitalRead(BTN_PIN) == LOW);
  unsigned long now = millis();

  if (reading && !isHolding) {
    btnPressStart = now;
    isHolding = true;
    isHoldUiActive = false;
    holdBeep3sDone = false;
    holdBeep5sDone = false;
  }
  else if (reading && isHolding) {
    unsigned long holdDur = now - btnPressStart;
    if (holdDur >= 400) {
      isHoldUiActive = true;
      if (holdDur >= 3000 && !holdBeep3sDone) { beepNonBlocking(2200, 50); holdBeep3sDone = true; }
      if (holdDur >= 5000 && !holdBeep5sDone) { beepNonBlocking(2600, 80); holdBeep5sDone = true; }
      if (now - lastHoldRenderTime >= 35) {
        lastHoldRenderTime = now;
        drawHoldProgressHUD(holdDur);
      }
    }
  }
  else if (!reading && isHolding) {
    unsigned long pressDur = now - btnPressStart;
    isHolding = false;

    if (isHoldUiActive) {
      isHoldUiActive = false;
      clickCount = 0;
      if (pressDur >= 5000) {
        soundClick();
        currentState = STATE_ABOUT;
        needRedraw = true;
      } else if (pressDur >= 3000) {
        executeCalibrationWizard();
        currentState = STATE_MAIN;
      } else {
        soundClick();
        needRedraw = true;       // ยกเลิก: วาดหน้าเดิมกลับมาทับ HUD
      }
      return;
    }
    else if (pressDur > 30 && pressDur < 400) {
      clickCount++;
      lastReleaseTime = now;
    }
  }

  if (clickCount > 0 && !isHolding && (now - lastReleaseTime > 320)) {
    if (currentState == STATE_ABOUT) {          // หน้าวิธีใช้: คลิกกี่ครั้งก็กลับหน้าหลัก
      soundClick();
      currentState = STATE_MAIN;
      needRedraw = true;
      clickCount = 0;
      return;
    }

    if (clickCount >= 4) {
      cycleSimulationMode();
    }
    else if (clickCount == 3) {
      resetAllStatistics();
    }
    else if (clickCount == 2) {
      isRunning = !isRunning;
      currentFlowRate_ml_hr = 0.0f;
      dropState = false;
      if (isRunning) {
        rateAnchorValid = false;               // ไม่นำช่วงที่พักไว้มาคำนวณอัตราไหล
        lastDropTimestamp = now;
        soundStart();
      } else {
        soundStop();
      }
      needRedraw = true;
    }
    else {
      currentPage = (currentPage % 3) + 1;
      needRedraw = true;
      soundClick();
    }
    clickCount = 0;
  }
}

// ============================================================================
// ไฟ LED บอกสถานะ: เขียว=ปกติ เหลือง=พักการนับ แดง=ไม่มีการไหล ม่วง=โหมดจำลอง
// ============================================================================
void updateStatusLed(unsigned long now) {
  if (ledFlashUntil > 0) {
    if (now < ledFlashUntil) return;
    ledFlashUntil = 0;
    lastLedCode = -1;
  }
  int code;
  if (!isRunning)          code = 1;
  else if (isNoFlow(now))  code = 2;
  else if (simEnabled())   code = 3;
  else                     code = 4;
  if (code == lastLedCode) return;
  lastLedCode = code;
  switch (code) {
    case 1:  setLedColor(50, 40, 0); break;
    case 2:  setLedColor(90, 0, 0);  break;
    case 3:  setLedColor(40, 0, 60); break;
    default: setLedColor(0, 35, 0);  break;
  }
}

// ============================================================================
// setup / loop
// ============================================================================
void drawSplashScreen() {
  tft.fillScreen(THEME_BG);
  tft.drawRoundRect(4, 4, 164, 312, 8, THEME_SKYBLUE);
  tft.drawRoundRect(6, 6, 160, 308, 6, THEME_PINK);

  drawCenteredString("SMART IV", 40, 3, THEME_SKYBLUE, THEME_BG);
  drawCenteredString("FLOW LAB", 70, 2, THEME_PINK, THEME_BG);

  tft.fillRoundRect(16, 118, 140, 56, 6, THEME_PINK);
  drawCenteredString("SIMULATION", 128, 2, COLOR_WHITE, THEME_PINK);
  drawCenteredString("& SENSOR TEST", 150, 1, COLOR_WHITE, THEME_PINK);

  drawCenteredString("Drop counting + Calibration", 196, 1, COLOR_WHITE, THEME_BG);
  drawCenteredString("Waveform & Trend graph", 214, 1, THEME_CYAN, THEME_BG);
  drawCenteredString("NO HOST / NO WIRELESS", 232, 1, COLOR_YELLOW, THEME_BG);
  drawCenteredString("Firmware v" APP_VERSION, 256, 1, THEME_SKYBLUE, THEME_BG);
  drawCenteredString("Hold 5 s = how to use", 284, 1, ST77XX_LIGHTGREY, THEME_BG);
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT); noTone(BUZZER_PIN);
  pinMode(TFT_BLK, OUTPUT); digitalWrite(TFT_BLK, HIGH);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(SENSOR_AO_PIN, INPUT);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  randomSeed((uint32_t)analogRead(SENSOR_AO_PIN) ^ micros());

  SPI_TFT.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(SCR_W, SCR_H);
  tft.setSPISpeed(40000000);
  tft.setRotation(0);
  tft.setTextWrap(false);

  // อ่านค่าคาลิเบรตเดิม (ใช้ namespace เดียวกับเฟิร์มแวร์เครื่องจริง จึงย้ายเครื่องมาทดสอบได้ทันที)
  labPrefs.begin("st_cal", true);
  valLiquid     = labPrefs.getInt("v_liq", 1200);
  valAir        = labPrefs.getInt("v_air", 2800);
  dropThreshold = labPrefs.getInt("th", 2000);
  hysteresis    = labPrefs.getInt("hys", 200);
  labPrefs.end();
  dropLowersAdc = (valLiquid < valAir);

  setLedColor(0, 40, 80);
  drawSplashScreen();
  playWelcomeMelody();
  delay(1200);

  unsigned long now = millis();
  recomputePlotRange();
  clearWaveBuffer();
  for (int i = 0; i < TREND_BARS; i++) { trendBin[i] = 0.0f; trendHas[i] = false; }
  lastBucketMs   = now;
  binStartMs     = now;
  lastRateSample = now;
  noiseWinStart  = now;
  noiseSubStart  = now;
  lastDropTimestamp = now;

  currentState = STATE_MAIN;
  currentPage  = PAGE_WAVE;
  needRedraw   = true;
}

void loop() {
  unsigned long now = millis();

  sampleSensorAndDetect(now);       // งานเร่งด่วนที่สุด เรียกทุกรอบเพื่อไม่ให้พลาดหยด
  handleButton();
  handleBuzzerEngine(now);
  updateFlowMath(now);
  updateTrendAccumulator(now);
  updateStatusLed(now);

  if (isHoldUiActive) return;       // ขณะกดค้าง ให้ HUD ครองหน้าจอ

  if (currentState == STATE_ABOUT) {
    if (needRedraw) drawAboutScreen();
    return;
  }

  if (needRedraw) {
    needRedraw = false;
    switch (currentPage) {
      case PAGE_WAVE:   drawWavePageFramework();   drawWavePageDynamic(now);   break;
      case PAGE_TREND:  drawTrendPageFramework();  drawTrendPageDynamic(now);  break;
      default:          drawSensorPageFramework(); drawSensorPageDynamic(now); break;
    }
    lastDynamicMs = now;
    return;
  }

  // ปรับค่าตัวเลข/กราฟตามรอบเวลาของแต่ละหน้า (กราฟรูปคลื่นวาดทีละคอลัมน์ในตัวอ่านเซนเซอร์แล้ว)
  unsigned long interval = (currentPage == PAGE_SENSOR) ? 150UL : 400UL;
  if (now - lastDynamicMs >= interval) {
    lastDynamicMs = now;
    switch (currentPage) {
      case PAGE_WAVE:   drawWavePageDynamic(now);   break;
      case PAGE_TREND:  drawTrendPageDynamic(now);  break;
      default:          drawSensorPageDynamic(now); break;
    }
  }
}
