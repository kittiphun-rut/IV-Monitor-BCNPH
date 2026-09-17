/**
 * ============================================================================
 * โครงการวิจัย: ผลของการใช้นวัตกรรม Smart IV Alert ต่อความแม่นยำในการแจ้งเตือนและปริมาณสารน้ำที่ได้รับ
 * หน่วยงาน: หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น อำเภอสูงเม่น จังหวัดแพร่
 * สถาบัน: วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
 *
 * ระบบ: Bed Station Node (เครื่องตรวจวัดและแจ้งเตือนประจำเตียง)
 * เวอร์ชัน: 7.5.0 (Protocol v3 — ใช้คู่กับ Central Host Firmware 4.6.0 / 4.7.0)
 * บอร์ดประมวลผล: ESP32-S3 Super Mini + จอสี TFT 1.47" (ST7789 SPI 172x320) + Passive Buzzer
 *
 * ---------------------------------------------------------------------------
 * เปลี่ยนใน V7.5.0: ออกแบบหน้าจอ TFT ใหม่ทั้งหมด (UX/UI สำหรับใช้งานจริงที่เตียง)
 *  - แนวคิด "IV BAG": หน้าหลักแสดงภาพถุงน้ำเกลือที่ระดับน้ำลดลงจริงตามปริมาณที่ให้ไปแล้ว
 *    พร้อมกระเปาะหยดที่มีภาพเคลื่อนไหวตามหยดจริง และตัวเลขสำคัญเพียง 4 ค่า
 *    (อัตราไหล / ปริมาณคงเหลือ / เวลาที่เหลือ / เวลาที่จะหมด)
 *  - ลดข้อความบนจอลงเหลือเท่าที่จำเป็น ใช้สีและตำแหน่งแทนคำอธิบาย
 *    แถบสถานะสีที่ขอบบนบอกภาวะของเตียงตั้งแต่แรกเห็น (เขียว/เหลือง/ส้ม/แดง/ฟ้า)
 *  - เพิ่มหน้าที่ 4: กราฟแท่งแนวโน้มอัตราไหลย้อนหลัง 30 นาที พร้อมเส้นอัตราเป้าหมาย
 *    ใช้ดูว่าการไหลสม่ำเสมอหรือมีช่วงที่ตกไป โดยไม่ต้องเปิดหน้าเว็บ
 *  - หน้า PLAN และ LINK จัดใหม่เป็นการ์ดตัวเลขใหญ่ อ่านได้จากปลายเตียง
 *  - จอเตือนวิกฤต/ใกล้หมดถุง/Screensaver ออกแบบใหม่ให้เหลือข้อความเท่าที่ต้องอ่าน
 *  - ตรรกะการวัด การสื่อสาร และการแจ้งเตือนทั้งหมดเหมือน V7.4.0 ทุกประการ
 *    (โครงสร้างแพ็กเก็ต Protocol v3 ไม่เปลี่ยน ใช้กับ Host เดิมได้ทันที)
 *
 * ---------------------------------------------------------------------------
 * เพิ่มใน V7.4.0: แจ้งเตือนใกล้หมด (เตรียมถุงใหม่)
 *  - เตือนเมื่อให้สารน้ำไปแล้วถึง % ที่ตั้งจากหน้าเว็บ (ค่าเริ่มต้น 80% ของปริมาตรตามแผน)
 *  - แสดงจอสีส้ม "PREPARE NEXT BAG" + เสียงเตือนเบา 3 ครั้ง ซ้ำทุก 5 นาทีจนกว่าจะกดรับทราบ
 *  - คลิกปุ่มที่เตียง หรือกดรับทราบที่ Host/หน้าเว็บ = รับทราบ (ซิงก์กันทั้งระบบ)
 *  - ไม่นับเป็นเหตุวิกฤต ไม่เข้าจอ EMERGENCY สีแดง
 *
 * สรุปการแก้ไขจาก V7.2.3
 *  1) โครงสร้างแพ็กเก็ต ESP-NOW ตรงกับ Host ทุกไบต์ (มี static_assert ตรวจขนาด)
 *     ส่งอัตราไหล + เวลาตั้งแต่หยดล่าสุด + แรงดันแบตเตอรี่ (ถ้ามีวงจร)
 *  2) รับ Drop factor จาก Host แทนค่าคงที่ 20 gtt/mL
 *  3) แก้แจ้งเตือนสายพับผิด: เดิมเข้า EMERGENCY ทันทีหลังหยดแรก และค้างเมื่อกดหยุดชั่วคราว
 *     -> ใช้เกณฑ์ "ไม่มีหยดนานเกิน 2.5 เท่าของช่วงหยดตามเป้าหมาย (ขั้นต่ำ 8 วินาที)" สูตรเดียวกับ Host
 *  4) รหัสเตือน เร็ว/ช้าเกิน/ครบแผน มาจาก Host (มีหน่วงยืนยัน 20 วินาที) — สายพับตัดสินในเครื่องทันที
 *  5) ค้นหาช่องสัญญาณของ Host อัตโนมัติ (Channel 1-13) เมื่อขาดการเชื่อมต่อ และจำช่องล่าสุด
 *  6) ลดการวาดจอ TFT ทุกรอบ loop -> วาดเฉพาะเมื่อสถานะเปลี่ยน ไม่แย่งเวลาอ่าน ADC ตรวจจับหยด
 *  7) รองรับคำสั่ง "เริ่มถุงใหม่" จากหน้าเว็บ (รีเซ็ตตัวนับ)
 *  8) ในจอ EMERGENCY: คลิก 1 ครั้ง = พักเสียง 2 นาที, ดับเบิลคลิก = หยุด/เริ่มนับ
 *  9) Screensaver ทำงานจริงเมื่อไม่มีการกดปุ่ม 3 นาที (คลิกเพื่อปลุก)
 * 10) ไฟ RGB แสดงสถานะ: แดง=เตือน, เหลือง=หยุดชั่วคราว, น้ำเงิน=ไม่พบ Host, เขียว=ปกติ
 *
 * การใช้ปุ่ม (ปุ่มเดียว)
 *  - คลิก 1 ครั้ง      : เปลี่ยนหน้า 1/2/3/4  (ในจอเตือน = พักเสียง 2 นาที)
 *  - ดับเบิลคลิก       : หยุด/เริ่มนับหยด (PAUSE)
 *  - คลิก 3 ครั้ง      : ตั้งหมายเลขเตียง 1-8
 *  - กดค้าง 3 วินาที   : Calibration Wizard
 *  - กดค้าง 5 วินาที   : หน้าผู้พัฒนา
 * ============================================================================
 */

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <time.h>
#include <sys/time.h>

#define APP_VERSION         "7.5.0"

#define TFT_CS              9
#define TFT_DC              10
#define TFT_RST             11
#define TFT_MOSI            12
#define TFT_SCLK            13
#define TFT_BLK             8

#define SENSOR_AO_PIN       4
#define BUZZER_PIN          3
#define BTN_PIN             2
#define RGB_LED_PIN         48

// วงจรวัดแบตเตอรี่ (ถ้ามี): ใส่หมายเลขขา ADC ที่ต่อผ่านตัวต้านทานแบ่งแรงดัน 1:1 เช่น 1
// -1 = ไม่มีวงจร -> ส่งค่า 0 และหน้าเว็บ/Host แสดง "N/A"
#define STATION_BAT_ADC_PIN -1
#define BAT_DIVIDER_RATIO   2.0f

// ----------------------------------------------------------------------------
// ค่าคงที่ของระบบสื่อสารและการแจ้งเตือน (ต้องตรงกับ Host)
// ----------------------------------------------------------------------------
#define DEFAULT_ESPNOW_CHANNEL  1
#define MAX_WIFI_CHANNEL        13
#define HOST_TIMEOUT_MS         5000
#define CHANNEL_SCAN_DWELL_MS   2500     // Host ส่ง Sync ทุก 1 วินาที -> รอแต่ละช่อง 2.5 วินาที
#define CHANNEL_SCAN_START_MS   6000     // หลังบูตรอช่องที่จำไว้ก่อน 6 วินาที
#define SEND_INTERVAL_MS        1000
#define MIN_OCCLUSION_MS        8000
#define MAX_OCCLUSION_MS        300000
#define SCREENSAVER_IDLE_MS     180000
#define SNOOZE_MS               120000
#define NEAR_END_REMIND_MS      300000   // เตือนใกล้หมดซ้ำทุก 5 นาทีจนกว่าจะรับทราบ
#define DEFAULT_NEAR_END_PCT    80

#define ALERT_NONE        0
#define ALERT_TOO_FAST    1
#define ALERT_TOO_SLOW    2
#define ALERT_NEAR_END    3   // ให้ไปแล้วถึง % ที่ตั้ง (เตรียมถุงใหม่)
#define ALERT_COMPLETE    4
#define ALERT_OCCLUSION   5

#define THEME_BG            0x0000
#define THEME_SKYBLUE       0x5DFF
#define THEME_CYAN          0x07FF
#define THEME_PINK          0xFD79
#define THEME_NAVY          0x09CD
#define THEME_DARKBOX       0x18C5
#define THEME_MAROON        0x6000

#define COLOR_WHITE         0xFFFF
#define COLOR_GREEN         0x07E0
#define COLOR_RED           0xF800
#define COLOR_ORANGE        0xFD20
#define COLOR_YELLOW        0xFFE0

// ชุดสีของหน้าจอรุ่นใหม่ (พื้นดำ การ์ดเทาเข้ม ตัวอักษรรองสีเทา)
#define UI_CARD             0x18C5   // พื้นการ์ด
#define UI_BAR              0x0A49   // แถบบนสุด
#define UI_LINE             0x39E7   // เส้นคั่น/แถบว่าง
#define UI_DIM              0x8410   // ข้อความรอง
#define UI_GREEN            0x2FEB   // เขียวอ่านง่ายบนพื้นดำ

#ifndef ST77XX_LIGHTGREY
  #define ST77XX_LIGHTGREY  0xC618
#endif
#ifndef ST77XX_DARKGREY
  #define ST77XX_DARKGREY   0x39E7
#endif

SPIClass SPI_TFT(FSPI);
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI_TFT, TFT_CS, TFT_DC, TFT_RST);
Preferences stationPrefs;

enum AppState {
  STATE_NORMAL_VIEW,
  STATE_EMERGENCY,
  STATE_SCREENSAVER,
  STATE_CREDIT,
  STATE_CONFIG_ID,
  STATE_NEAR_END_NOTICE
};

// ---- สถานะที่ใช้เลือกสีและคำบนจอ ----
// ต้องประกาศไว้ตอนต้นไฟล์ เพราะ Arduino IDE แทรก prototype ของฟังก์ชันไว้ก่อนส่วนแสดงผล
enum UiStatus { UI_OK = 0, UI_FAST, UI_SLOW, UI_NOFLOW, UI_NEAREND, UI_DONE, UI_PAUSED };

AppState currentState       = STATE_NORMAL_VIEW;
int currentNursePage        = 1;
uint8_t currentStationId    = 1;
uint8_t tempConfigStationId = 1;
unsigned long configAutoSaveTimeout = 0;
unsigned long lastUserActivityTime  = 0;

bool isRunning              = true;
bool isClockSynced          = false;
volatile bool isHostOnline  = false;
volatile int lastHostRssi   = -100;
volatile unsigned long lastHostRecvTime = 0;
bool stateNeedsRedraw       = true;

int valLiquid               = 1200;
int valAir                  = 2800;
int dropThreshold           = 2000;
int hysteresis              = 200;
bool dropState              = false;

// ----------------------------------------------------------------------------
// โครงสร้างข้อมูลรับ-ส่ง ESP-NOW (Protocol v2) — ต้องเหมือนกับ Host ทุกไบต์
// ----------------------------------------------------------------------------
typedef struct __attribute__((packed)) struct_message {
  uint8_t  stationId;
  uint8_t  isRunning;
  uint32_t totalDrops;
  uint32_t periodDrops;
  float    flowRateHr;
  uint32_t msSinceLastDrop;
  float    batteryVolts;
  uint8_t  flags;            // bit0 = พยาบาลกดรับทราบเตือนใกล้หมดที่เตียง
} struct_message;            // 23 bytes

typedef struct __attribute__((packed)) struct_host_sync {
  uint32_t epochTime;
  uint8_t  isSynced;
  uint8_t  stationId;
  float    targetRateHr;
  float    totalPlanMl;
  uint8_t  alertCode;
  uint8_t  dropFactor;
  uint8_t  resetSeq;
  uint8_t  hostChannel;
  uint8_t  nearEndPct;       // % ของปริมาตรตามแผนที่เริ่มเตือนใกล้หมด
  uint8_t  flags;            // bit0 = Host บันทึกว่ารับทราบเตือนใกล้หมดแล้ว
} struct_host_sync;          // 20 bytes

static_assert(sizeof(struct_message) == 23, "struct_message must be 23 bytes (match Host)");
static_assert(sizeof(struct_host_sync) == 20, "struct_host_sync must be 20 bytes (match Host)");

struct_message myData;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint32_t totalDrops                 = 0;
uint32_t periodDropsCounter         = 0;

float currentFlowRate_ml_hr         = 0.0;
float currentGttMin                 = 0.0;
float totalVolumeMl                 = 0.0;

// ค่าจาก Host (0 = Host ยังไม่ได้ตั้งค่า)
volatile float targetRateHr         = 0.0;
volatile float totalPlanMl          = 0.0;
volatile uint8_t hostAlertCode      = ALERT_NONE;
volatile uint8_t dropFactor         = 20;
volatile uint8_t hostReportedChannel = 0;
volatile bool pendingCounterReset   = false;
volatile uint8_t nearEndPct         = DEFAULT_NEAR_END_PCT;
volatile bool hostNearEndAck        = false;   // Host ยืนยันว่ารับทราบแล้ว
bool nearEndAckRequest              = false;   // กดรับทราบที่เตียง รอ Host ยืนยัน
unsigned long nearNextChimeTime     = 0;
uint8_t nearChimeRemaining          = 0;
int16_t knownResetSeq               = -1;

uint8_t espnowChannel               = DEFAULT_ESPNOW_CHANNEL;
uint8_t savedChannel                = DEFAULT_ESPNOW_CHANNEL;
unsigned long lastChannelHopTime    = 0;

unsigned long lastDropTimestamp     = 0;
bool hasFirstDropOccurred           = false;
bool skipNextInterval               = false;

unsigned long lastSendTime          = 0;
unsigned long lastDebounceTime      = 0;
unsigned long snoozeUntilTime       = 0;
const unsigned long minDropInterval = 55;

unsigned long buzzerBeepUntil       = 0;
unsigned long buzzerNextBeepTime    = 0;

bool isDropFalling                  = false;
unsigned long dropFallStartTime     = 0;
int animDropY                       = 190;
int animDropPrevY                   = 190;
int8_t animModeDrawn                = -1;
unsigned long lastAnimFrame         = 0;
unsigned long poolFlashUntil        = 0;
const int CHAMBER_NOZZLE_Y          = 190;   // ตำแหน่งปลายหยดในกระเปาะ (หน้าจอใหม่)
const int CHAMBER_POOL_Y            = 204;   // ผิวน้ำในกระเปาะ
const unsigned long DROP_FALL_DURATION = 160;

bool isHoldUiActive                 = false;
bool holdBeep3sDone                 = false;
bool holdBeep5sDone                 = false;
unsigned long lastHoldRenderTime    = 0;

// ---- รหัสเตือนที่หน้าจอใช้ (อัปเดตทุกรอบใน loop) ----
uint8_t uiAlertCode = ALERT_NONE;

// ---- แคชของหน้าจอ: วาดใหม่เฉพาะเมื่อค่าที่แสดงเปลี่ยนจริง ----
String cacheClock = "";
int    cacheBars = -9999;
int    cacheStatusBar = -1;
bool   cacheBedChip = false;
int    cacheRate = -9999, cacheTarget = -9999, cacheLeftMl = -9999;
int    cachePct = -999, cacheBagPct = -999;
String cacheTimeLeft = "", cacheEndClock = "", cacheStatusWord = "";
String cacheP2a = "", cacheP2b = "", cacheP2c = "", cacheP2d = "", cacheP2foot = "";
String cacheP3a = "", cacheP3b = "", cacheP3c = "";
String cacheP4 = "", cacheNear = "", cacheConfig = "";
uint8_t lastEmergencyCode = 255;

// ---- กราฟแนวโน้มอัตราไหล: 30 แท่ง แท่งละ 1 นาที = ย้อนหลัง 30 นาที ----
#define TREND_BARS   30
#define TREND_BIN_MS 60000UL
float    trendBin[TREND_BARS];
bool     trendHas[TREND_BARS];
int      trendHead      = 0;
float    trendSum       = 0.0f;
int      trendSamples   = 0;
float    trendLive      = 0.0f;
float    trendScale     = 100.0f;
bool     trendNeedsRedraw = true;
unsigned long trendBinStart   = 0;
unsigned long trendLastSample = 0;

// ---- ประกาศล่วงหน้า (ฟังก์ชันที่ถูกเรียกก่อนบรรทัดที่นิยามไว้) ----
void beepNonBlocking(uint16_t freq, uint16_t durationMs);
void resetTrendData();
void updateTrendAccumulator(unsigned long now);

// ----------------------------------------------------------------------------
// ฟังก์ชันช่วย
// ----------------------------------------------------------------------------
uint8_t safeDropFactor(uint8_t df) {
  return (df == 10 || df == 15 || df == 20 || df == 60) ? df : 20;
}

bool isCriticalAlert(uint8_t code) {
  return code == ALERT_TOO_FAST || code == ALERT_TOO_SLOW ||
         code == ALERT_COMPLETE || code == ALERT_OCCLUSION;
}

// สูตรเดียวกับ Host
uint32_t occlusionThresholdMs() {
  float rate = targetRateHr;
  if (rate <= 0.0f) return MIN_OCCLUSION_MS;
  float dropsPerHr = rate * (float)safeDropFactor(dropFactor);
  uint32_t expected = (uint32_t)(3600000.0f / dropsPerHr);
  uint32_t th = (expected * 5) / 2;
  if (th < MIN_OCCLUSION_MS) th = MIN_OCCLUSION_MS;
  if (th > MAX_OCCLUSION_MS) th = MAX_OCCLUSION_MS;
  return th;
}

bool isLocallyOccluded() {
  return isRunning && hasFirstDropOccurred && (millis() - lastDropTimestamp > occlusionThresholdMs());
}

uint8_t safeNearPct(uint8_t p) {
  return (p >= 50 && p <= 95) ? p : DEFAULT_NEAR_END_PCT;
}

bool isNearEndReached() {
  if (totalPlanMl <= 0) return false;
  return totalVolumeMl >= totalPlanMl * (float)safeNearPct(nearEndPct) / 100.0f;
}

bool isNearEndAcknowledged() {
  return hostNearEndAck || nearEndAckRequest;
}

// รหัสเตือนที่ใช้จริงในเครื่องนี้
uint8_t effectiveAlertCode() {
  if (!isRunning) return ALERT_NONE;
  if (isLocallyOccluded()) return ALERT_OCCLUSION;           // ตัดสินทันทีในเครื่อง
  if (isHostOnline) {
    uint8_t c = hostAlertCode;
    if (c == ALERT_OCCLUSION) c = ALERT_NONE;                // ข้อมูลในเครื่องใหม่กว่า
    return c;
  }
  if (totalPlanMl > 0 && totalVolumeMl >= totalPlanMl) return ALERT_COMPLETE;
  if (isNearEndReached()) return ALERT_NEAR_END;
  return ALERT_NONE;
}

bool isNearEndPending(uint8_t code) {
  return code == ALERT_NEAR_END && !isNearEndAcknowledged();
}

void acknowledgeNearEnd() {
  nearEndAckRequest = true;
  nearChimeRemaining = 0;
  noTone(BUZZER_PIN);
  beepNonBlocking(2600, 40);
}

float readStationBattery() {
#if STATION_BAT_ADC_PIN >= 0
  long sum = 0;
  for (int i = 0; i < 8; i++) sum += analogRead(STATION_BAT_ADC_PIN);
  float v = ((sum / 8.0f) / 4095.0f) * 3.3f * BAT_DIVIDER_RATIO;
  return (v < 2.5f) ? 0.0f : v;
#else
  return 0.0f;
#endif
}

void drawCenteredString(const String &str, int y, uint8_t size, uint16_t color, uint16_t bg) {
  int16_t x1, y1;
  uint16_t w, h;
  tft.setTextSize(size);
  tft.getTextBounds(str, 0, y, &x1, &y1, &w, &h);
  int x = (172 - (int)w) / 2;
  if (x < 0) x = 0;
  tft.setTextColor(color, bg);
  tft.setCursor(x, y);
  tft.print(str);
}

void setLedColor(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_LED_PIN, r, g, b);
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

void soundClick()       { beepNonBlocking(2600, 20); }
void soundStart()       { beepNonBlocking(2000, 70); }
void soundStop()        { beepNonBlocking(1000, 140); }
void soundScreensaver() { beepNonBlocking(1800, 40); }

void playWelcomeMelody() {
  int notes[] = { 1046, 1318, 1568, 2093 };
  for (int i = 0; i < 4; i++) {
    modalSafeBeep(notes[i], 80);
    delay(25);
  }
}

void handlePassiveBuzzerEngine(uint8_t code) {
  unsigned long now = millis();
  if (buzzerBeepUntil > 0 && now >= buzzerBeepUntil) {
    noTone(BUZZER_PIN);
    buzzerBeepUntil = 0;
  }

  // เสียงเตือนใกล้หมด: 3 ครั้งสั้น ๆ แล้วเงียบ 5 นาที (ดังเฉพาะเมื่อยังไม่มีเหตุวิกฤต)
  if (!isCriticalAlert(code)) {
    if (!isNearEndPending(code)) { nearChimeRemaining = 0; nearNextChimeTime = 0; return; }
    if (nearChimeRemaining == 0 && (nearNextChimeTime == 0 || now >= nearNextChimeTime)) {
      nearChimeRemaining = 3;
      buzzerNextBeepTime = now;
      nearNextChimeTime = now + NEAR_END_REMIND_MS;
    }
    if (nearChimeRemaining > 0 && now >= buzzerNextBeepTime) {
      beepNonBlocking(2000, 70);
      buzzerNextBeepTime = now + 220;
      nearChimeRemaining--;
    }
    return;
  }

  if (now < snoozeUntilTime) return;
  if (now < buzzerNextBeepTime) return;

  switch (code) {
    case ALERT_OCCLUSION: beepNonBlocking(1800, 150); buzzerNextBeepTime = now + 900;  break;
    case ALERT_TOO_FAST:  beepNonBlocking(2200, 80);  buzzerNextBeepTime = now + 700;  break;
    case ALERT_TOO_SLOW:  beepNonBlocking(1200, 120); buzzerNextBeepTime = now + 1200; break;
    case ALERT_COMPLETE:  beepNonBlocking(1500, 250); buzzerNextBeepTime = now + 1500; break;
  }
}

void updateStatusLed(uint8_t code) {
  static int lastLed = -1;
  int led;
  if (currentState == STATE_CONFIG_ID) led = 4;
  else if (isCriticalAlert(code) && millis() >= snoozeUntilTime) led = 0;
  else if (isNearEndPending(code)) led = 5;
  else if (!isRunning) led = 1;
  else if (!isHostOnline) led = 2;
  else led = 3;

  if (led == lastLed) return;
  lastLed = led;
  switch (led) {
    case 0: setLedColor(90, 0, 0);  break;
    case 1: setLedColor(60, 40, 0); break;
    case 2: setLedColor(0, 0, 60);  break;
    case 3: setLedColor(0, 30, 0);  break;
    case 4: setLedColor(50, 0, 50); break;
    case 5: setLedColor(90, 35, 0); break;
  }
}

void drawHoldProgressHUD(unsigned long holdDur) {
  int boxX = 6, boxY = 100, boxW = 160, boxH = 100;
  tft.fillRoundRect(boxX, boxY, boxW, boxH, 8, THEME_NAVY);
  tft.drawRoundRect(boxX, boxY, boxW, boxH, 8, THEME_CYAN);

  int barX = boxX + 15;
  int barY = boxY + 48;
  int barW = 130;
  int barH = 12;

  if (holdDur < 3000) {
    int fill = map(holdDur, 400, 3000, 0, barW);
    if (fill < 0) fill = 0;
    if (fill > barW) fill = barW;

    drawCenteredString("BUTTON HOLDING", boxY + 12, 1, THEME_SKYBLUE, THEME_NAVY);
    drawCenteredString("-> CALIBRATE (3s)", boxY + 28, 1, COLOR_YELLOW, THEME_NAVY);
    tft.drawRect(barX, barY, barW, barH, COLOR_WHITE);
    tft.fillRect(barX + 1, barY + 1, fill, barH - 2, THEME_PINK);
    drawCenteredString("Release <3s = Cancel", boxY + 82, 1, ST77XX_LIGHTGREY, THEME_NAVY);
  }
  else if (holdDur < 5000) {
    int fill = map(holdDur, 3000, 5000, 0, barW);
    if (fill < 0) fill = 0;
    if (fill > barW) fill = barW;

    drawCenteredString("RELEASE = CALIB", boxY + 12, 1, COLOR_GREEN, THEME_NAVY);
    drawCenteredString("-> CREDITS (5s)", boxY + 28, 1, THEME_PINK, THEME_NAVY);
    tft.drawRect(barX, barY, barW, barH, COLOR_WHITE);
    tft.fillRect(barX + 1, barY + 1, fill, barH - 2, THEME_CYAN);
  }
  else {
    drawCenteredString(">> CREDITS READY <<", boxY + 14, 1, COLOR_GREEN, THEME_NAVY);
    drawCenteredString("RELEASE TO VIEW", boxY + 30, 1, COLOR_YELLOW, THEME_NAVY);
    tft.drawRect(barX, barY, barW, barH, COLOR_WHITE);
    tft.fillRect(barX + 1, barY + 1, barW - 2, barH - 2, COLOR_GREEN);
  }
}

int sampleSensorAveragePrecise(int samples = 40) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(SENSOR_AO_PIN);
    delayMicroseconds(500);
  }
  return (int)(sum / samples);
}

int waitAndMonitorSensorStep(const char* stepTitle, const char* stepDescTh, const char* stepDescEn, uint16_t boxColor) {
  noTone(BUZZER_PIN);
  buzzerBeepUntil = 0;

  tft.fillRoundRect(6, 36, 160, 160, 8, THEME_DARKBOX);
  tft.drawRoundRect(6, 36, 160, 160, 8, boxColor);
  drawCenteredString(stepTitle, 44, 1, COLOR_YELLOW, THEME_DARKBOX);
  drawCenteredString(stepDescEn, 60, 2, COLOR_WHITE, THEME_DARKBOX);
  // หมายเหตุ: ฟอนต์ Adafruit GFX ไม่มีอักษรไทย จึงแสดงคำอธิบายภาษาอังกฤษบนจอ
  (void)stepDescTh;

  tft.fillRoundRect(20, 102, 132, 44, 6, THEME_NAVY);
  tft.drawRoundRect(20, 102, 132, 44, 6, boxColor);
  drawCenteredString("SENSOR LIVE ADC", 106, 1, THEME_SKYBLUE, THEME_NAVY);
  drawCenteredString("Click Button >>", 176, 1, COLOR_WHITE, THEME_DARKBOX);

  unsigned long lastAdcUpdate = 0;
  int lastVal = -1;
  while (digitalRead(BTN_PIN) == HIGH) {
    if (millis() - lastAdcUpdate >= 60) {
      lastAdcUpdate = millis();
      int liveAdc = analogRead(SENSOR_AO_PIN);
      if (liveAdc != lastVal) {
        lastVal = liveAdc;
        tft.fillRect(24, 120, 124, 20, THEME_NAVY);
        drawCenteredString(String(liveAdc), 120, 2, COLOR_WHITE, THEME_NAVY);
      }
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

void executeButtonCalibrationWizard() {
  noTone(BUZZER_PIN);
  buzzerBeepUntil = 0;

  tft.fillScreen(THEME_BG);
  tft.fillRect(0, 0, 172, 28, THEME_PINK);
  drawCenteredString("CALIBRATION WIZARD", 8, 1, COLOR_WHITE, THEME_PINK);

  int newLiquid = waitAndMonitorSensorStep("STEP 1 OF 2", "(กระเปาะมีสารน้ำ)", "FILL LIQUID", THEME_SKYBLUE);
  int newAir    = waitAndMonitorSensorStep("STEP 2 OF 2", "(กระเปาะเปล่าไม่มีน้ำ)", "EMPTY CHAMBER", THEME_PINK);

  int diff = abs(newLiquid - newAir);
  tft.fillRoundRect(6, 204, 160, 108, 8, THEME_NAVY);
  tft.drawRoundRect(6, 204, 160, 108, 8, (diff >= 80) ? COLOR_GREEN : COLOR_RED);

  if (diff >= 80) {
    valLiquid     = newLiquid;
    valAir        = newAir;
    dropThreshold = (valLiquid + valAir) / 2;
    hysteresis    = (int)(diff * 0.20);
    if (hysteresis < 60)  hysteresis = 60;
    if (hysteresis > 300) hysteresis = 300;

    stationPrefs.begin("st_cal", false);
    stationPrefs.putInt("v_liq", valLiquid);
    stationPrefs.putInt("v_air", valAir);
    stationPrefs.putInt("th", dropThreshold);
    stationPrefs.putInt("hys", hysteresis);
    stationPrefs.end();

    drawCenteredString("CALIBRATION OK!", 212, 1, COLOR_GREEN, THEME_NAVY);
    drawCenteredString("TH:" + String(dropThreshold) + " HYS:" + String(hysteresis), 232, 1, COLOR_WHITE, THEME_NAVY);
    modalSafeBeep(2000, 100);
    delay(100);
    modalSafeBeep(2500, 150);
    delay(1500);
  } else {
    // ไม่เขียนทับค่าเดิมเมื่อ Calibrate ไม่ผ่าน
    drawCenteredString("FAILED: DIFF LOW!", 216, 1, COLOR_RED, THEME_NAVY);
    drawCenteredString("Keep previous values", 236, 1, COLOR_WHITE, THEME_NAVY);
    modalSafeBeep(800, 200);
    delay(1500);
  }
  // ตรวจจับหยดหยุดทำงานระหว่าง Wizard -> ไม่ให้นับเป็นสายพับทันที
  if (hasFirstDropOccurred) { lastDropTimestamp = millis(); skipNextInterval = true; }
  lastUserActivityTime = millis();
  stateNeedsRedraw = true;
}

// ----------------------------------------------------------------------------
// ESP-NOW Receive Callback (รับ Sync จาก Host)
// ----------------------------------------------------------------------------
#if defined(ESP_IDF_VERSION) && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
void OnTimeSyncRecv(const esp_now_recv_info_t *info, const uint8_t *incomingDataBytes, int len) {
  int rssi = (info && info->rx_ctrl) ? info->rx_ctrl->rssi : -55;
#else
void OnTimeSyncRecv(const uint8_t *mac, const uint8_t *incomingDataBytes, int len) {
  int rssi = -55;
#endif
  if (len != sizeof(struct_host_sync)) return;   // แพ็กเก็ตของ Station อื่น (22 bytes) จะถูกกรองออก

  struct_host_sync syncData;
  memcpy(&syncData, incomingDataBytes, sizeof(syncData));
  lastHostRssi = rssi;
  lastHostRecvTime = millis();
  isHostOnline = true;
  hostReportedChannel = syncData.hostChannel;

  if (syncData.epochTime > 1600000000) {
    struct timeval tv = { (time_t)syncData.epochTime, 0 };
    settimeofday(&tv, NULL);
    isClockSynced = (syncData.isSynced == 1);
  }

  if (syncData.stationId == currentStationId) {
    targetRateHr  = syncData.targetRateHr;
    totalPlanMl   = syncData.totalPlanMl;
    hostAlertCode = syncData.alertCode;
    dropFactor    = safeDropFactor(syncData.dropFactor);
    nearEndPct    = safeNearPct(syncData.nearEndPct);
    hostNearEndAck = (syncData.flags & 0x01) != 0;
    if (hostNearEndAck) nearEndAckRequest = false;   // Host ยืนยันแล้ว

    if (knownResetSeq < 0) {
      knownResetSeq = syncData.resetSeq;          // ครั้งแรกหลังบูต: รับค่าโดยไม่รีเซ็ต
    } else if (syncData.resetSeq != (uint8_t)knownResetSeq) {
      knownResetSeq = syncData.resetSeq;
      pendingCounterReset = true;                  // ให้ loop() เป็นผู้รีเซ็ต
    }
  }
}

void applyCounterReset() {
  pendingCounterReset = false;
  totalDrops = 0;
  periodDropsCounter = 0;
  totalVolumeMl = 0;
  currentFlowRate_ml_hr = 0;
  currentGttMin = 0;
  hasFirstDropOccurred = false;
  skipNextInterval = false;
  isDropFalling = false;
  snoozeUntilTime = 0;
  hostAlertCode = ALERT_NONE;
  hostNearEndAck = false;
  nearEndAckRequest = false;
  nearNextChimeTime = 0;
  nearChimeRemaining = 0;
  if (currentState == STATE_EMERGENCY || currentState == STATE_NEAR_END_NOTICE) currentState = STATE_NORMAL_VIEW;
  resetTrendData();                 // เริ่มถุงใหม่ = เริ่มกราฟแนวโน้มใหม่
  stateNeedsRedraw = true;
  beepNonBlocking(2400, 120);
}

// ----------------------------------------------------------------------------
// ค้นหาช่องสัญญาณของ Host
// ----------------------------------------------------------------------------
void setEspNowChannel(uint8_t ch) {
  if (esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE) == ESP_OK) {
    espnowChannel = ch;
  }
}

void manageChannelHunting(unsigned long now) {
  if (isHostOnline) {
    if (espnowChannel != savedChannel) {
      savedChannel = espnowChannel;
      stationPrefs.begin("st_cfg", false);
      stationPrefs.putUChar("ch", savedChannel);
      stationPrefs.end();
    }
    return;
  }
  if (now < CHANNEL_SCAN_START_MS) return;
  if (currentState == STATE_CONFIG_ID) return;

  if (now - lastChannelHopTime >= CHANNEL_SCAN_DWELL_MS) {
    lastChannelHopTime = now;
    uint8_t next = (espnowChannel % MAX_WIFI_CHANNEL) + 1;
    setEspNowChannel(next);
  }
}

// ----------------------------------------------------------------------------
// ตรวจจับหยดน้ำ
// ----------------------------------------------------------------------------
void checkAnalogDrop() {
  if (!isRunning) return;

  int sensorValue = analogRead(SENSOR_AO_PIN);
  unsigned long now = millis();

  if (!dropState && (sensorValue < dropThreshold)) {
    if (now - lastDebounceTime > minDropInterval) {
      dropState = true;
      totalDrops++;
      periodDropsCounter++;
      lastDebounceTime = now;

      isDropFalling = true;
      dropFallStartTime = now;
      animDropY = CHAMBER_NOZZLE_Y;

      uint8_t df = safeDropFactor(dropFactor);
      if (hasFirstDropOccurred && !skipNextInterval) {
        unsigned long dropDeltaMs = now - lastDropTimestamp;
        if (dropDeltaMs > minDropInterval && dropDeltaMs < MAX_OCCLUSION_MS) {
          float instantGttMin   = 60000.0f / (float)dropDeltaMs;
          float instantRateMlHr = (instantGttMin * 60.0f) / (float)df;

          if (currentFlowRate_ml_hr == 0.0f) {
            currentFlowRate_ml_hr = instantRateMlHr;
          } else {
            currentFlowRate_ml_hr = (instantRateMlHr * 0.75f) + (currentFlowRate_ml_hr * 0.25f);
          }
        }
      }
      hasFirstDropOccurred = true;
      skipNextInterval = false;
      lastDropTimestamp = now;
      currentGttMin = (currentFlowRate_ml_hr * (float)df) / 60.0f;
      totalVolumeMl = (float)totalDrops / (float)df;
    }
  }
  else if (dropState && (sensorValue > (dropThreshold + hysteresis))) {
    dropState = false;
  }
}

void togglePause() {
  isRunning = !isRunning;
  currentFlowRate_ml_hr = 0.0f;
  currentGttMin = 0.0f;
  if (isRunning) {
    soundStart();
    // เริ่มนับเวลาใหม่หลังกลับมาทำงาน ไม่ให้ช่วงที่หยุดถูกนับเป็นสายพับหรือใช้คำนวณ rate
    if (hasFirstDropOccurred) {
      lastDropTimestamp = millis();
      skipNextInterval = true;
    }
  } else {
    soundStop();
    noTone(BUZZER_PIN);
  }
  stateNeedsRedraw = true;
}

String getStationClockStr() {
  if (!isClockSynced) return String("--:--");
  time_t now;
  time(&now);
  struct tm *timeinfo = localtime(&now);
  char buf[12];
  strftime(buf, sizeof(buf), "%H:%M", timeinfo);
  return String(buf);
}

// ============================================================================
// ส่วนแสดงผล (UI v2) — เน้นอ่านง่ายจากปลายเตียง ใช้ข้อความเท่าที่จำเป็น
//   หน้า 1 IV BAG  : ภาพถุงน้ำเกลือ + กระเปาะหยด + ตัวเลขสำคัญ 4 ค่า
//   หน้า 2 PLAN    : แผนการให้สารน้ำและเวลา (ตัวเลขใหญ่ 4 ค่า)
//   หน้า 3 LINK    : สถานะการเชื่อมต่อและเซนเซอร์
//   หน้า 4 TREND   : กราฟแท่งแนวโน้มอัตราไหลย้อนหลัง 30 นาที
// ทุกหน้าวาดเฉพาะส่วนที่ค่าเปลี่ยน เพื่อไม่ให้ SPI แย่งเวลาการอ่านเซนเซอร์
// ============================================================================

UiStatus uiStatusOf(uint8_t code) {
  if (!isRunning) return UI_PAUSED;
  switch (code) {
    case ALERT_OCCLUSION: return UI_NOFLOW;
    case ALERT_TOO_FAST:  return UI_FAST;
    case ALERT_TOO_SLOW:  return UI_SLOW;
    case ALERT_COMPLETE:  return UI_DONE;
    case ALERT_NEAR_END:  return UI_NEAREND;
    default:              return UI_OK;
  }
}

uint16_t uiStatusColor(UiStatus s) {
  switch (s) {
    case UI_OK:      return UI_GREEN;
    case UI_FAST:    return COLOR_ORANGE;
    case UI_SLOW:    return COLOR_YELLOW;
    case UI_NOFLOW:  return COLOR_RED;
    case UI_NEAREND: return COLOR_ORANGE;
    case UI_DONE:    return THEME_CYAN;
    default:         return THEME_SKYBLUE;
  }
}

const char* uiStatusWord(UiStatus s) {
  switch (s) {
    case UI_OK:      return "NORMAL";
    case UI_FAST:    return "TOO FAST";
    case UI_SLOW:    return "TOO SLOW";
    case UI_NOFLOW:  return "NO FLOW";
    case UI_NEAREND: return "NEXT BAG";
    case UI_DONE:    return "COMPLETE";
    default:         return "PAUSED";
  }
}

// ---- ตัวช่วยวางข้อความ (ฟอนต์มาตรฐาน: 1 ตัวอักษร = 6xsize กว้าง, 8xsize สูง) ----
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

// เติมช่องว่างท้ายข้อความให้ยาวคงที่ เพื่อเขียนทับของเดิมได้โดยไม่ต้องล้างพื้นที่ (ไม่กะพริบ)
String padTo(const String &s, unsigned int width) {
  String out = s;
  while (out.length() < width) out += ' ';
  return out;
}

// ---- ค่าที่ใช้ร่วมกันหลายหน้า ----
int remainingMlInt() {
  float r = totalPlanMl - totalVolumeMl;
  if (r < 0) r = 0;
  return (int)(r + 0.5f);
}

int infusedPct() {
  if (totalPlanMl <= 0) return -1;
  int p = (int)(totalVolumeMl * 100.0f / totalPlanMl + 0.5f);
  return (p > 100) ? 100 : p;
}

// นาทีที่เหลือจนหมดถุง (-1 = คำนวณไม่ได้)
int minutesLeft() {
  if (totalPlanMl <= 0) return -1;
  if (!isRunning || isLocallyOccluded()) return -1;
  float calcRate = (currentFlowRate_ml_hr > 5.0f) ? currentFlowRate_ml_hr : (float)targetRateHr;
  if (calcRate <= 0) return -1;
  return (int)((remainingMlInt() / calcRate) * 60.0f);
}

String timeLeftText(bool compact) {
  int m = minutesLeft();
  if (m < 0) return compact ? String("--h--m") : String("--h --m");
  char b[16];
  snprintf(b, sizeof(b), compact ? "%dh%02dm" : "%dh %02dm", m / 60, m % 60);
  return String(b);
}

String endClockText() {
  int m = minutesLeft();
  if (!isClockSynced || m < 0) return String("--:--");
  time_t now;
  time(&now);
  time_t fin = now + ((time_t)m * 60);
  char buf[8];
  strftime(buf, sizeof(buf), "%H:%M", localtime(&fin));
  return String(buf);
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

// ============================================================================
// หน้า 1: IV BAG — ภาพถุงน้ำเกลือ + กระเปาะหยด + ตัวเลขสำคัญ
// ============================================================================
#define BAG_X       12
#define BAG_Y       34
#define BAG_W       58
#define BAG_H       124
#define CH_X        26
#define CH_Y        176
#define CH_W        30
#define CH_H        46
#define DROP_X      41
#define RIGHT_X     84

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

void updatePage2Dynamic() {
  drawTopBar(false);

  String v1 = (targetRateHr > 0) ? String((int)(targetRateHr + 0.5f)) : String("--");
  String v2 = (totalPlanMl > 0) ? String((int)totalPlanMl) : String("--");
  String v3 = String((int)totalVolumeMl);
  String v4 = endClockText();

  if (v1 != cacheP2a) { cacheP2a = v1; drawStatCard(30,  "", v1, THEME_SKYBLUE, false); }
  if (v2 != cacheP2b) { cacheP2b = v2; drawStatCard(84,  "", v2, COLOR_WHITE,   false); }
  if (v3 != cacheP2c) { cacheP2c = v3; drawStatCard(138, "", v3, COLOR_WHITE,   false); }
  if (v4 != cacheP2d) { cacheP2d = v4; drawStatCard(192, "", v4, UI_GREEN,      false); }

  String foot = timeLeftText(false) + " left";
  String foot2 = (totalPlanMl > 0)
      ? ("NEXT BAG AT " + String(safeNearPct(nearEndPct)) + "%")
      : String("NEXT BAG AT --");
  String key = foot + foot2;
  if (key != cacheP2foot) {
    cacheP2foot = key;
    textCenterIn(padTo(foot, 12), 6, 160, 248, 2, COLOR_WHITE, THEME_BG);
    textCenterIn(padTo(foot2, 20), 6, 160, 274, 1, COLOR_ORANGE, THEME_BG);
  }
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

  int adc = analogRead(SENSOR_AO_PIN);
  String sensorKey = String(adc / 4) + ":" + String(dropThreshold) + ":" + String(hysteresis);
  if (sensorKey != cacheP3b) {
    cacheP3b = sensorKey;
    textAt(padTo("ADC " + String(adc), 10), 14, 118, 2, dropState ? COLOR_YELLOW : COLOR_WHITE, UI_CARD);
    textAt(padTo("TH " + String(dropThreshold) + "   HYS " + String(hysteresis), 22), 14, 144, 1, UI_DIM, UI_CARD);
    textAt(padTo("LIQ " + String(valLiquid) + "   AIR " + String(valAir), 22), 14, 160, 1, UI_DIM, UI_CARD);
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

// ============================================================================
// หน้า 4: TREND — กราฟแท่งอัตราไหลย้อนหลัง 30 นาที (แท่งละ 1 นาที)
// ============================================================================
#define CHART_X    12
#define CHART_BARW 5
#define CHART_TOP  56
#define CHART_BOT  196

float trendValueAt(int i) {
  int idx = (trendHead + i) % TREND_BARS;
  return trendHas[idx] ? trendBin[idx] : 0.0f;
}

bool trendValidAt(int i) {
  int idx = (trendHead + i) % TREND_BARS;
  return trendHas[idx];
}

float niceScale(float v) {
  float s = 20.0f;
  while (s < v && s < 400.0f) s += (s < 100.0f) ? 10.0f : (s < 200.0f ? 20.0f : 50.0f);
  return s;
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

void updatePage4Dynamic() {
  drawTopBar(false);

  float sum = 0.0f;
  int cnt = 0;
  for (int i = 1; i < TREND_BARS; i++) {
    if (!trendValidAt(i)) continue;
    sum += trendValueAt(i);
    cnt++;
  }
  int avg = (cnt > 0) ? (int)(sum / cnt + 0.5f) : -1;
  int now = isRunning ? (int)(currentFlowRate_ml_hr + 0.5f) : -1;

  String key = String(now) + ":" + String(avg);
  if (key != cacheP4) {
    cacheP4 = key;
    textAt(padTo(now >= 0 ? String(now) : String("--"), 4), 14, 234, 3, COLOR_WHITE, UI_CARD);
    textAt(padTo(avg >= 0 ? String(avg) : String("--"), 4), 100, 238, 2, THEME_CYAN, UI_CARD);
  }
  if (trendNeedsRedraw) drawTrendChart();
  else                  drawTrendLiveBar();
}

// ============================================================================
// เก็บข้อมูลกราฟแนวโน้ม (ทำงานตลอดเวลา ไม่ว่าจะเปิดหน้าไหนอยู่)
// ============================================================================
void resetTrendData() {
  for (int i = 0; i < TREND_BARS; i++) { trendBin[i] = 0.0f; trendHas[i] = false; }
  trendHead = 0;
  trendSum = 0.0f;
  trendSamples = 0;
  trendLive = 0.0f;
  trendBinStart = millis();
  trendNeedsRedraw = true;
}

void updateTrendAccumulator(unsigned long now) {
  if (now - trendLastSample >= 1000) {
    trendLastSample = now;
    trendSum += isRunning ? currentFlowRate_ml_hr : 0.0f;
    trendSamples++;
    trendLive = (trendSamples > 0) ? (trendSum / trendSamples) : 0.0f;
  }
  if (now - trendBinStart >= TREND_BIN_MS) {
    trendBinStart = now;
    trendBin[trendHead] = (trendSamples > 0) ? (trendSum / trendSamples) : 0.0f;
    trendHas[trendHead] = true;
    trendHead = (trendHead + 1) % TREND_BARS;
    trendSum = 0.0f;
    trendSamples = 0;
    trendLive = 0.0f;
    trendNeedsRedraw = true;
  }
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

void updateNearEndNoticeDynamic() {
  int pct = infusedPct();
  int left = remainingMlInt();
  String end = endClockText();
  String key = String(pct) + ":" + String(left) + ":" + end;
  if (key == cacheNear) return;
  cacheNear = key;

  textCenterIn(padTo("INFUSED " + String(pct < 0 ? 0 : pct) + "%", 12), 12, 148, 126, 2, COLOR_WHITE, THEME_BG);
  textCenterIn(padTo(String(left) + " mL LEFT", 12), 12, 148, 152, 2, COLOR_ORANGE, THEME_BG);
  textCenterIn(padTo("EMPTY AT " + end, 18), 12, 148, 180, 1, COLOR_YELLOW, THEME_BG);
  drawProgressBar(26, 196, 120, 10, (pct < 0) ? 0 : pct, COLOR_ORANGE);
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

void drawStationIdConfigFramework() {
  tft.fillScreen(THEME_BG);
  tft.fillRoundRect(6, 16, 160, 34, 6, THEME_SKYBLUE);
  drawCenteredString("SET BED ID", 26, 2, THEME_BG, THEME_SKYBLUE);
  tft.fillRoundRect(6, 62, 160, 150, 8, UI_CARD);
  drawCenteredString("CLICK = NEXT (1-8)", 224, 1, UI_DIM, THEME_BG);
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
  drawCenteredString(padTo("SAVING IN " + String(leftSec) + "s", 16), 250, 1, COLOR_YELLOW, THEME_BG);
}

// ----------------------------------------------------------------------------
// ปุ่มกด
// ----------------------------------------------------------------------------
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
    lastUserActivityTime = now;
  }
  else if (reading && isHolding) {
    unsigned long holdDur = now - btnPressStart;
    if (holdDur >= 400 && currentState != STATE_CONFIG_ID) {
      isHoldUiActive = true;
      if (holdDur >= 3000 && !holdBeep3sDone) {
        beepNonBlocking(2200, 50);
        holdBeep3sDone = true;
      }
      if (holdDur >= 5000 && !holdBeep5sDone) {
        beepNonBlocking(2600, 80);
        holdBeep5sDone = true;
      }
      if (now - lastHoldRenderTime >= 35) {
        lastHoldRenderTime = now;
        drawHoldProgressHUD(holdDur);
      }
    }
  }
  else if (!reading && isHolding) {
    unsigned long pressDur = now - btnPressStart;
    isHolding = false;
    lastUserActivityTime = now;

    if (isHoldUiActive) {
      isHoldUiActive = false;
      clickCount = 0;
      if (pressDur >= 5000) {
        soundScreensaver();
        currentState = STATE_CREDIT;
        stateNeedsRedraw = true;
      } else if (pressDur >= 3000) {
        executeButtonCalibrationWizard();
      } else {
        soundClick();
        stateNeedsRedraw = true;
      }
      return;
    }
    else if (pressDur > 30 && pressDur < 400) {
      clickCount++;
      lastReleaseTime = now;
    }
  }

  if (clickCount > 0 && !isHolding && (now - lastReleaseTime > 280)) {
    int clicks = clickCount;
    clickCount = 0;

    if (currentState == STATE_CONFIG_ID) {
      soundClick();
      tempConfigStationId = (tempConfigStationId % 8) + 1;
      configAutoSaveTimeout = now + 4000;
      return;
    }

    if (currentState == STATE_EMERGENCY) {
      if (clicks >= 2) {
        togglePause();                         // หยุดนับ เช่น ขณะเปลี่ยนถุง/แก้สายพับ
      } else {
        snoozeUntilTime = now + SNOOZE_MS;
        noTone(BUZZER_PIN);
        buzzerBeepUntil = 0;
        soundClick();
      }
      currentState = STATE_NORMAL_VIEW;
      stateNeedsRedraw = true;
      return;
    }

    if (currentState == STATE_NEAR_END_NOTICE) {
      acknowledgeNearEnd();                    // คลิกกี่ครั้งก็ได้ = รับทราบ
      currentState = STATE_NORMAL_VIEW;
      currentNursePage = 1;
      stateNeedsRedraw = true;
      return;
    }

    if (currentState == STATE_CREDIT) {
      soundClick();
      currentState = STATE_NORMAL_VIEW;
      stateNeedsRedraw = true;
      return;
    }

    if (clicks >= 3) {
      soundClick();
      noTone(BUZZER_PIN);
      tempConfigStationId = currentStationId;
      configAutoSaveTimeout = now + 4000;
      currentState = STATE_CONFIG_ID;
      stateNeedsRedraw = true;
    }
    else if (clicks == 2) {
      togglePause();
      if (currentState == STATE_SCREENSAVER) currentState = STATE_NORMAL_VIEW;
    }
    else {
      soundClick();
      if (currentState == STATE_SCREENSAVER) {
        currentState = STATE_NORMAL_VIEW;
      } else {
        currentNursePage = (currentNursePage % 4) + 1;
      }
      stateNeedsRedraw = true;
    }
  }
}

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  setenv("TZ", "ICT-7", 1);
  tzset();

  pinMode(BUZZER_PIN, OUTPUT); noTone(BUZZER_PIN);
  pinMode(TFT_BLK, OUTPUT); digitalWrite(TFT_BLK, HIGH);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(SENSOR_AO_PIN, INPUT);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  SPI_TFT.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(172, 320);
  tft.setSPISpeed(40000000);
  tft.setRotation(0);
  tft.setTextWrap(false);

  stationPrefs.begin("st_cfg", false);
  currentStationId = stationPrefs.getUChar("id", 1);
  savedChannel     = stationPrefs.getUChar("ch", DEFAULT_ESPNOW_CHANNEL);
  stationPrefs.end();
  if (currentStationId < 1 || currentStationId > 8) currentStationId = 1;
  if (savedChannel < 1 || savedChannel > MAX_WIFI_CHANNEL) savedChannel = DEFAULT_ESPNOW_CHANNEL;

  stationPrefs.begin("st_cal", true);
  valLiquid     = stationPrefs.getInt("v_liq", 1200);
  valAir        = stationPrefs.getInt("v_air", 2800);
  dropThreshold = stationPrefs.getInt("th", 2000);
  hysteresis    = stationPrefs.getInt("hys", 200);
  stationPrefs.end();

  setLedColor(0, 40, 80);
  resetTrendData();
  trendLastSample = millis();

  tft.fillScreen(THEME_BG);
  drawCenteredString("SMART IV", 60, 3, THEME_SKYBLUE, THEME_BG);
  drawCenteredString("ALERT", 92, 2, THEME_PINK, THEME_BG);

  char bedBuf[12];
  snprintf(bedBuf, sizeof(bedBuf), "BED %02d", currentStationId);
  tft.fillRoundRect(26, 140, 120, 48, 8, THEME_SKYBLUE);
  drawCenteredString(String(bedBuf), 154, 3, THEME_BG, THEME_SKYBLUE);

  drawCenteredString("Bed Station Node", 214, 1, UI_DIM, THEME_BG);
  drawCenteredString("FW v" APP_VERSION, 232, 1, THEME_SKYBLUE, THEME_BG);
  drawCenteredString("BCN Phrae Innovation", 256, 1, UI_DIM, THEME_BG);

  playWelcomeMelody();
  delay(600);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);
  setEspNowChannel(savedChannel);

  if (esp_now_init() != ESP_OK) {
    drawCenteredString("ESP-NOW INIT FAILED", 270, 1, COLOR_RED, THEME_BG);
    Serial.println("[STATION] ESP-NOW init FAILED");
  } else {
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnTimeSyncRecv));

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;              // 0 = ใช้ช่องปัจจุบัน (รองรับการเปลี่ยนช่องระหว่างค้นหา Host)
    peerInfo.ifidx = WIFI_IF_STA;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
  }

  currentState = STATE_NORMAL_VIEW;
  currentNursePage = 1;
  stateNeedsRedraw = true;
  lastUserActivityTime = millis();
  lastChannelHopTime = millis();
}

// ----------------------------------------------------------------------------
// Loop
// ----------------------------------------------------------------------------
void loop() {
  checkAnalogDrop();
  handleButton();

  unsigned long currentMillis = millis();

  if (pendingCounterReset) applyCounterReset();

  // ตรวจสอบการเชื่อมต่อกับ Host หากเกิน 5 วินาทีให้ถือว่าหลุด
  unsigned long lr = lastHostRecvTime;
  if (lr == 0 || (currentMillis > lr && currentMillis - lr > HOST_TIMEOUT_MS)) {
    isHostOnline = false;
  }
  manageChannelHunting(currentMillis);

  // สายพับ -> อัตราไหลเป็น 0
  if (isLocallyOccluded() && currentFlowRate_ml_hr != 0.0f) {
    currentFlowRate_ml_hr = 0.0f;
    currentGttMin = 0.0f;
  }
  totalVolumeMl = (float)totalDrops / (float)safeDropFactor(dropFactor);

  uint8_t alertCode = effectiveAlertCode();
  uiAlertCode = alertCode;                                // ให้ทุกหน้าจอใช้รหัสเดียวกัน
  updateTrendAccumulator(currentMillis);                  // เก็บข้อมูลกราฟตลอดเวลา
  if (!isCriticalAlert(alertCode)) snoozeUntilTime = 0;   // เหตุการณ์หายไปแล้ว ยกเลิก snooze
  handlePassiveBuzzerEngine(alertCode);
  updateStatusLed(alertCode);

  // ---- ส่งข้อมูลให้ Host ทุก 1 วินาที (ส่งแม้อยู่ในหน้าตั้งค่า) ----
  if (currentMillis - lastSendTime >= SEND_INTERVAL_MS) {
    myData.stationId       = currentStationId;
    myData.isRunning       = isRunning ? 1 : 0;
    myData.totalDrops      = totalDrops;
    myData.periodDrops     = periodDropsCounter;
    myData.flowRateHr      = isRunning ? currentFlowRate_ml_hr : 0.0f;
    myData.msSinceLastDrop = hasFirstDropOccurred ? (uint32_t)(currentMillis - lastDropTimestamp) : 0;
    if (myData.msSinceLastDrop == 0 && hasFirstDropOccurred) myData.msSinceLastDrop = 1;
    myData.batteryVolts    = readStationBattery();
    myData.flags           = nearEndAckRequest ? 0x01 : 0x00;

    if (esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      periodDropsCounter = 0;
    }
    lastSendTime = currentMillis;
  }

  if (currentState == STATE_CONFIG_ID) {
    if (stateNeedsRedraw) drawStationIdConfigFramework();
    updateStationIdConfigDynamic();

    if (currentMillis >= configAutoSaveTimeout) {
      if (tempConfigStationId != currentStationId) {
        currentStationId = tempConfigStationId;
        stationPrefs.begin("st_cfg", false);
        stationPrefs.putUChar("id", currentStationId);
        stationPrefs.end();
        // ล้างค่าของเตียงเดิม รอค่าของเตียงใหม่จาก Host
        targetRateHr = 0;
        totalPlanMl = 0;
        hostAlertCode = ALERT_NONE;
        knownResetSeq = -1;
      }
      soundStart();
      currentState = STATE_NORMAL_VIEW;
      stateNeedsRedraw = true;
    }
    return;
  }

  // ---- จัดการสถานะเตือน ----
  bool critical = isCriticalAlert(alertCode);
  if (critical && currentMillis >= snoozeUntilTime) {
    if (currentState != STATE_EMERGENCY && !isHoldUiActive) {
      currentState = STATE_EMERGENCY;
      stateNeedsRedraw = true;
    } else if (currentState == STATE_EMERGENCY && alertCode != lastEmergencyCode) {
      stateNeedsRedraw = true;
    }
  } else if (currentState == STATE_EMERGENCY) {
    currentState = STATE_NORMAL_VIEW;
    stateNeedsRedraw = true;
  }

  // ---- เตือนใกล้หมด (ไม่ใช่เหตุวิกฤต) ----
  if (!critical && isNearEndPending(alertCode) && !isHoldUiActive &&
      (currentState == STATE_NORMAL_VIEW || currentState == STATE_SCREENSAVER)) {
    currentState = STATE_NEAR_END_NOTICE;
    stateNeedsRedraw = true;
  } else if (currentState == STATE_NEAR_END_NOTICE && (critical || !isNearEndPending(alertCode))) {
    currentState = critical ? STATE_EMERGENCY : STATE_NORMAL_VIEW;
    stateNeedsRedraw = true;
  }

  // ---- Screensaver เมื่อไม่มีการใช้งาน ----
  if (currentState == STATE_NORMAL_VIEW && !isHoldUiActive &&
      currentMillis - lastUserActivityTime >= SCREENSAVER_IDLE_MS) {
    currentState = STATE_SCREENSAVER;
    stateNeedsRedraw = true;
  }

  if (!isHoldUiActive) {
    if (currentState == STATE_EMERGENCY) {
      if (stateNeedsRedraw) { drawEmergencyScreen(alertCode); updateEmergencyDynamic(); }
    }
    else if (currentState == STATE_NORMAL_VIEW) {
      if (currentNursePage == 1) {
        if (stateNeedsRedraw) { drawPage1Framework(); updatePage1Dynamic(); }
        updateDripAnimation();
      } else if (currentNursePage == 2) {
        if (stateNeedsRedraw) { drawPage2Framework(); updatePage2Dynamic(); }
      } else if (currentNursePage == 3) {
        if (stateNeedsRedraw) { drawPage3Framework(); updatePage3Dynamic(); }
      } else if (currentNursePage == 4) {
        if (stateNeedsRedraw) { drawPage4Framework(); updatePage4Dynamic(); }
      }
    }
    else if (currentState == STATE_SCREENSAVER) {
      if (stateNeedsRedraw) { drawScreensaverFramework(); updateScreensaverDynamic(); }
    }
    else if (currentState == STATE_NEAR_END_NOTICE) {
      if (stateNeedsRedraw) { drawNearEndNoticeFramework(); updateNearEndNoticeDynamic(); }
    }
    else if (currentState == STATE_CREDIT) {
      if (stateNeedsRedraw) drawDeveloperCreditScreen();
    }
  }

  static unsigned long lastDynamicUpdate = 0;
  if (currentMillis - lastDynamicUpdate >= 1000) {
    lastDynamicUpdate = currentMillis;

    if (!isHoldUiActive) {
      if (currentState == STATE_NORMAL_VIEW) {
        if (currentNursePage == 1) updatePage1Dynamic();
        else if (currentNursePage == 2) updatePage2Dynamic();
        else if (currentNursePage == 3) updatePage3Dynamic();
        else if (currentNursePage == 4) updatePage4Dynamic();
      } else if (currentState == STATE_SCREENSAVER) {
        updateScreensaverDynamic();
      } else if (currentState == STATE_EMERGENCY) {
        updateEmergencyDynamic();
      } else if (currentState == STATE_NEAR_END_NOTICE) {
        updateNearEndNoticeDynamic();
      }
    }
  }
}
